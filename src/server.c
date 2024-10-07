#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include "my_socket.h"
#include "logger.h" 
#include "globals.h"

FILE *log_file;
char client_usernames[MAX_CLIENTS][50];
char message_history[NUM_CHANNELS][MAX_MESSAGES][BUFFER_SIZE];
int message_count[NUM_CHANNELS] = {0};
pthread_mutex_t messages_mutex = PTHREAD_MUTEX_INITIALIZER;

// Função para verificar se o nome de usuário já existe
int is_username_taken(const char *username) {
    for (int i = 0; i < client_count; ++i) {
        if (strcmp(client_usernames[i], username) == 0) {
            return 1;
        }
    }
    return 0;
}

// Função para tratar cada cliente conectado
void *handle_client(void *arg) {
    int client_socket = *(int *)arg;
    char buffer[BUFFER_SIZE];
    int bytes_read;
    
    // Receber nome de usuário do cliente
    char username[50];
    while (1) {
        if ((bytes_read = recv(client_socket, username, 50, 0)) <= 0) {
            perror("Erro ao receber nome do usuário");
            write_log(log_file, "Erro ao receber nome do usuário.");
            close(client_socket);
            return NULL;
        }
        username[bytes_read] = '\0';

        pthread_mutex_lock(&clients_mutex);
        if (is_username_taken(username)) {
            // Se o nome de usuário já estiver em uso, enviar mensagem de erro
            send(client_socket, "username_in_use", strlen("username_in_use"), 0);
            pthread_mutex_unlock(&clients_mutex);
        } else {
            // Nome de usuário único, pode prosseguir
            strcpy(client_usernames[client_count], username);
            pthread_mutex_unlock(&clients_mutex);
            break;
        }
    }

    printf("%s se conectou.\n", username);
    char log_msg[BUFFER_SIZE + 200];

    // Receber escolha de canal
    int channel; 
    if ((bytes_read = recv(client_socket, buffer, BUFFER_SIZE, 0)) <= 0) {
        perror("Erro ao receber canal");
        write_log(log_file, "Erro ao receber canal.");
        close(client_socket);
        return NULL;
    }
    
    channel = atoi(buffer) - 1;
    
    if (channel < 0 || channel >= NUM_CHANNELS) {
        printf("Canal inválido escolhido por %s.\n", username);
        snprintf(log_msg, sizeof(log_msg), "Canal inválido escolhido por %s.", username);
        write_log(log_file, log_msg);
        close(client_socket);
        return NULL;
    }

    printf("%s entrou no canal %d.\n", username, channel + 1);
    snprintf(log_msg, sizeof(log_msg), "Usuário %s entrou no canal %d.", username, channel + 1);
    write_log(log_file, log_msg);

    pthread_mutex_lock(&clients_mutex);
    client_channels[client_count] = channel;
    clients[client_count] = client_socket;
    int client_index = client_count;
    client_count++;
    pthread_mutex_unlock(&clients_mutex);

    // Enviar histórico de mensagens do canal atual
    pthread_mutex_lock(&messages_mutex);
    for (int i = 0; i < message_count[channel]; ++i) {
        if (send(client_socket, message_history[channel][i], strlen(message_history[channel][i]), 0) < 0) {
            perror("Erro ao enviar histórico de mensagens");
            write_log(log_file, "Erro ao enviar histórico de mensagens.");
        }
    }
    pthread_mutex_unlock(&messages_mutex);

    // Loop para receber mensagens do cliente
    while ((bytes_read = recv(client_socket, buffer, BUFFER_SIZE, 0)) > 0) {
        buffer[bytes_read] = '\0';

        // Verificar se o cliente deseja mudar de canal
        if (strstr(buffer, "change_channel") == buffer) {
            // Cliente deseja mudar de canal
            int new_channel = atoi(buffer + 15) - 1;  // Extrai o novo canal do comando
            if (new_channel < 0 || new_channel >= NUM_CHANNELS) {
                printf("Canal inválido escolhido por %s.\n", username);
                continue;
            }
            printf("\n%s mudou para o canal %d.", username, new_channel + 1);

            // Atualizar o canal do cliente
            pthread_mutex_lock(&clients_mutex);
            client_channels[client_index] = new_channel;
            pthread_mutex_unlock(&clients_mutex);

            // Enviar o histórico do novo canal
            pthread_mutex_lock(&messages_mutex);
            for (int i = 0; i < message_count[new_channel]; ++i) {
                if (send(client_socket, message_history[new_channel][i], strlen(message_history[new_channel][i]), 0) < 0) {
                    perror("Erro ao enviar histórico de mensagens");
                    write_log(log_file, "Erro ao enviar histórico de mensagens.");
                }
            }
            pthread_mutex_unlock(&messages_mutex);

            continue;
        }

        printf("\nMensagem recebida de %s no canal %d: %s", username, channel + 1, buffer);

        snprintf(log_msg, sizeof(log_msg), "Mensagem recebida de %s no canal %d: %s", username, channel + 1, buffer);
        write_log(log_file, log_msg);

        char message_with_username[BUFFER_SIZE + 50];
        if (snprintf(message_with_username, sizeof(message_with_username), "%s: %s", username, buffer) >= sizeof(message_with_username)) {
            fprintf(stderr, "Mensagem truncada! Nome de usuário ou mensagem muito grande.\n");
            write_log(log_file, "Mensagem truncada! Nome de usuário ou mensagem muito grande.");
        }

        pthread_mutex_lock(&messages_mutex);
        if (message_count[channel] < MAX_MESSAGES) {
            strncpy(message_history[channel][message_count[channel]], message_with_username, BUFFER_SIZE);
            message_count[channel]++;
        }
        pthread_mutex_unlock(&messages_mutex);

        broadcast_message(message_with_username, client_socket, client_channels[client_index], client_index);
    }

    // Se ocorrer um erro ou o cliente se desconectar
    if (bytes_read == 0) {
        printf("%s se desconectou do canal %d.\n", username, channel + 1);
        snprintf(log_msg, sizeof(log_msg), "Usuário %s se desconectou do canal %d.", username, channel + 1);
        write_log(log_file, log_msg);
    } else {
        perror("Erro ao receber dados do cliente");
        write_log(log_file, "Erro ao receber dados do cliente.");
    }

    close(client_socket);

    pthread_mutex_lock(&clients_mutex);
    for (int i = 0; i < client_count; ++i) {
        if (clients[i] == client_socket) {
            for (int j = i; j < client_count - 1; ++j) {
                clients[j] = clients[j + 1];
                client_channels[j] = client_channels[j + 1];
                strncpy(client_usernames[j], client_usernames[j + 1], sizeof(client_usernames[j]));
            }
            break;
        }
    }
    client_count--;
    pthread_mutex_unlock(&clients_mutex);

    return NULL;
}

int main() {
    int server_socket, client_socket;
    struct sockaddr_in server_addr, client_addr;

    log_file = init_log();
    write_log(log_file, "Servidor iniciado.");

    server_socket = my_socket();
    my_bind(server_socket, &server_addr, PORT);
    my_listen(server_socket, MAX_CLIENTS);

    printf("Servidor iniciado na porta %d\n", PORT);
    write_log(log_file, "Servidor escutando por conexões...");

    while (1) {
        client_socket = my_accept(server_socket, &client_addr);
        if (client_socket < 0) {
            continue;
        }

        write_log(log_file, "Novo cliente conectado.");

        pthread_t tid;
        if (pthread_create(&tid, NULL, handle_client, &client_socket) != 0) {
            perror("Erro ao criar thread");
            write_log(log_file, "Erro ao criar thread.");
        }
    }

    close(server_socket);
    close_log(log_file); 
    return 0;
}

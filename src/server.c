#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include "my_socket.h"
#include "logger.h" 

#define PORT 8883
#define MAX_CLIENTS 10
#define BUFFER_SIZE 1024
#define MAX_MESSAGES 100
#define NUM_CHANNELS 5

#define ANSI_COLOR_BLUE "\x1b[34m"
#define ANSI_COLOR_RESET "\x1b[0m"
#define ANSI_COLOR_USER1 "\x1b[32m"
#define ANSI_COLOR_USER2 "\x1b[33m"
#define ANSI_COLOR_USER3 "\x1b[35m"
#define ANSI_COLOR_USER4 "\x1b[36m"
#define ANSI_COLOR_USER5 "\x1b[31m"

FILE *log_file;
int clients[MAX_CLIENTS];
int client_channels[MAX_CLIENTS];
char client_usernames[MAX_CLIENTS][50];
int client_count = 0;
char message_history[NUM_CHANNELS][MAX_MESSAGES][BUFFER_SIZE];
int message_count[NUM_CHANNELS] = {0};
pthread_mutex_t clients_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t messages_mutex = PTHREAD_MUTEX_INITIALIZER;

const char *get_user_color(int client_index) {
    switch (client_index % 5) {
        case 0: return ANSI_COLOR_USER1;
        case 1: return ANSI_COLOR_USER2;
        case 2: return ANSI_COLOR_USER3;
        case 3: return ANSI_COLOR_USER4;
        case 4: return ANSI_COLOR_USER5;
        default: return ANSI_COLOR_RESET;
    }
}

// Função para enviar uma mensagem para todos os clientes do mesmo canal, exceto o remetente
void broadcast_message(char *message, int sender_fd, int channel, int sender_index) {
    pthread_mutex_lock(&clients_mutex);
    for (int i = 0; i < client_count; ++i) {
        if (clients[i] != sender_fd && client_channels[i] == channel) {
            char colored_message[BUFFER_SIZE + 50];
            const char *color = get_user_color(sender_index);
            snprintf(colored_message, sizeof(colored_message), "%s%s%s", color, message, ANSI_COLOR_RESET);
            if (send(clients[i], colored_message, strlen(colored_message), 0) < 0) {
                perror("Erro ao enviar mensagem");
                write_log(log_file, "Erro ao enviar mensagem.");
                continue;
            }
        }
    }
    pthread_mutex_unlock(&clients_mutex);
}

// Função para tratar cada cliente conectado
void *handle_client(void *arg) {
    int client_socket = *(int *)arg;
    char buffer[BUFFER_SIZE];
    int bytes_read;
    
    // Receber nome de usuário do cliente
    char username[50];
    if ((bytes_read = recv(client_socket, username, 50, 0)) <= 0) {
        perror("Erro ao receber nome do usuário");
        write_log(log_file, "Erro ao receber nome do usuário.");
        close(client_socket);
        return NULL;
    }
    username[bytes_read] = '\0';

    pthread_mutex_lock(&clients_mutex);
    for (int i = 0; i < client_count; ++i) {
        if (strcmp(client_usernames[i], username) == 0) {
            snprintf(username + strlen(username), sizeof(username) - strlen(username), "_%d", client_count);
            break;
        }
    }
    pthread_mutex_unlock(&clients_mutex);

    printf("%s se conectou.\n", username);
    char log_msg[BUFFER_SIZE + 200];

    // Receber escolha de canal (declarar 'channel' antes do primeiro uso)
    int channel;  // Declarar a variável antes de usá-la
    
    if ((bytes_read = recv(client_socket, buffer, BUFFER_SIZE, 0)) <= 0) {
        perror("Erro ao receber canal");
        write_log(log_file, "Erro ao receber canal.");
        close(client_socket);
        return NULL;
    }
    
    channel = atoi(buffer) - 1;  // Inicializar 'channel' aqui
    
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
    strncpy(client_usernames[client_count], username, sizeof(client_usernames[client_count]) - 1);
    client_usernames[client_count][sizeof(client_usernames[client_count]) - 1] = '\0';
    int client_index = client_count;
    client_count++;
    pthread_mutex_unlock(&clients_mutex);

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
        printf("Mensagem recebida de %s no canal %d: %s", username, channel + 1, buffer);

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

        broadcast_message(message_with_username, client_socket, channel, client_index);
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

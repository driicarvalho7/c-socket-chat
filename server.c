#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include "my_socket.h"

#define PORT 8881
#define MAX_CLIENTS 10
#define BUFFER_SIZE 1024
#define MAX_MESSAGES 100
#define NUM_CHANNELS 5

int clients[MAX_CLIENTS];
int client_channels[MAX_CLIENTS];
int client_count = 0;
char message_history[NUM_CHANNELS][MAX_MESSAGES][BUFFER_SIZE];
int message_count[NUM_CHANNELS] = {0};
pthread_mutex_t clients_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t messages_mutex = PTHREAD_MUTEX_INITIALIZER;

void broadcast_message(char *message, int sender_fd, int channel) {
    pthread_mutex_lock(&clients_mutex);
    for (int i = 0; i < client_count; ++i) {
        if (clients[i] != sender_fd && client_channels[i] == channel) {
            if (send(clients[i], message, strlen(message), 0) < 0) {
                perror("Erro ao enviar mensagem");
                continue;
            }
        }
    }
    pthread_mutex_unlock(&clients_mutex);
}

void *handle_client(void *arg) {
    int client_socket = *(int *)arg;
    char buffer[BUFFER_SIZE];
    int bytes_read;

    char username[50];
    if ((bytes_read = recv(client_socket, username, 50, 0)) <= 0) {
        perror("Erro ao receber nome do usuário");
        close(client_socket);
        return NULL;
    }
    username[bytes_read] = '\0';
    printf("%s se conectou.\n", username);

    // Escolha do canal
    char channel_msg[] = "";
    if (send(client_socket, channel_msg, strlen(channel_msg), 0) < 0) {
        perror("Erro ao enviar mensagem de escolha de canal");
        close(client_socket);
        return NULL;
    }

    if ((bytes_read = recv(client_socket, buffer, BUFFER_SIZE, 0)) <= 0) {
        perror("Erro ao receber canal");
        close(client_socket);
        return NULL;
    }
    int channel = atoi(buffer) - 1;
    if (channel < 0 || channel >= NUM_CHANNELS) {
        printf("Canal inválido escolhido por %s.\n", username);
        close(client_socket);
        return NULL;
    }
    printf("%s entrou no canal %d.\n", username, channel + 1);

    pthread_mutex_lock(&clients_mutex);
    client_channels[client_count] = channel;
    clients[client_count++] = client_socket;
    pthread_mutex_unlock(&clients_mutex);

    // Enviar histórico de mensagens do canal para o cliente recém-conectado
    pthread_mutex_lock(&messages_mutex);
    for (int i = 0; i < message_count[channel]; ++i) {
        if (send(client_socket, message_history[channel][i], strlen(message_history[channel][i]), 0) < 0) {
            perror("Erro ao enviar histórico de mensagens");
        }
    }
    pthread_mutex_unlock(&messages_mutex);

    while ((bytes_read = recv(client_socket, buffer, BUFFER_SIZE, 0)) > 0) {
        buffer[bytes_read] = '\0';
        printf("Mensagem recebida de %s no canal %d: %s", username, channel + 1, buffer);

        char message_with_username[BUFFER_SIZE + 50];
        if (snprintf(message_with_username, sizeof(message_with_username), "%s: %s", username, buffer) >= sizeof(message_with_username)) {
            fprintf(stderr, "Mensagem truncada! Nome de usuário ou mensagem muito grande.\n");
        }

        pthread_mutex_lock(&messages_mutex);
        if (message_count[channel] < MAX_MESSAGES) {
            strncpy(message_history[channel][message_count[channel]], message_with_username, BUFFER_SIZE);
            message_count[channel]++;
        }
        pthread_mutex_unlock(&messages_mutex);

        broadcast_message(message_with_username, client_socket, channel);
    }

    close(client_socket);
    pthread_mutex_lock(&clients_mutex);
    for (int i = 0; i < client_count; ++i) {
        if (clients[i] == client_socket) {
            for (int j = i; j < client_count - 1; ++j) {
                clients[j] = clients[j + 1];
                client_channels[j] = client_channels[j + 1];
            }
            break;
        }
    }
    client_count--;
    pthread_mutex_unlock(&clients_mutex);
    printf("%s se desconectou do canal %d.\n", username, channel + 1);
    return NULL;
}

int main() {
    int server_socket, client_socket;
    struct sockaddr_in server_addr, client_addr;

    server_socket = my_socket();
    my_bind(server_socket, &server_addr, PORT);
    my_listen(server_socket, MAX_CLIENTS);

    printf("Servidor iniciado na porta %d\n", PORT);

    while (1) {
        client_socket = my_accept(server_socket, &client_addr);
        if (client_socket < 0) {
            continue;
        }

        pthread_t tid;
        if (pthread_create(&tid, NULL, handle_client, &client_socket) != 0) {
            perror("Erro ao criar thread");
        }
    }

    close(server_socket);
    return 0;
}

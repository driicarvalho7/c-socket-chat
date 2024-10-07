#include "my_socket.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>
#include "globals.h"

// Variáveis globais de controle de clientes (normalmente estariam em outro arquivo para modularidade)
extern int clients[MAX_CLIENTS];
extern int client_channels[MAX_CLIENTS];
extern int client_count;
extern pthread_mutex_t clients_mutex;

// Função para criar o socket
int my_socket() {
    int socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_fd < 0) {
        perror("Erro ao criar socket");
        exit(EXIT_FAILURE);
    }
    return socket_fd;
}

// Função para fazer o bind do socket ao endereço e porta
void my_bind(int socket_fd, struct sockaddr_in *address, int port) {
    address->sin_family = AF_INET;
    address->sin_addr.s_addr = INADDR_ANY; // Aceitar conexões de qualquer IP
    address->sin_port = htons(port);       // Converte a porta para o formato de rede

    // Faz o bind do socket
    if (bind(socket_fd, (struct sockaddr *)address, sizeof(*address)) < 0) {
        perror("Erro no bind");
        close(socket_fd);
        exit(EXIT_FAILURE);
    }
}

// Função para colocar o socket em modo de escuta
void my_listen(int socket_fd, int max_clients) {
    if (listen(socket_fd, max_clients) < 0) {
        perror("Erro no listen");
        close(socket_fd);
        exit(EXIT_FAILURE);
    }
}

// Função para aceitar conexões de clientes
int my_accept(int socket_fd, struct sockaddr_in *client_addr) {
    socklen_t client_len = sizeof(*client_addr);
    int client_socket = accept(socket_fd, (struct sockaddr *)client_addr, &client_len);
    if (client_socket < 0) {
        perror("Erro ao aceitar conexão");
        return -1;
    }
    return client_socket;
}

// Função para conectar o cliente ao servidor
void my_connect(int socket_fd, struct sockaddr_in *server_addr, const char *ip, int port) {
    server_addr->sin_family = AF_INET;
    server_addr->sin_port = htons(port);

    // Valida o IP antes de tentar conectar
    if (inet_pton(AF_INET, ip, &server_addr->sin_addr) <= 0) {
        fprintf(stderr, "Erro: Endereço IP inválido\n");
        close(socket_fd);
        exit(EXIT_FAILURE);
    }

    // Tenta conectar ao servidor
    if (connect(socket_fd, (struct sockaddr *)server_addr, sizeof(*server_addr)) < 0) {
        perror("Erro ao conectar ao servidor");
        close(socket_fd);
        exit(EXIT_FAILURE);
    }
}

// Função para enviar uma mensagem para todos os clientes do mesmo canal, exceto o remetente
void broadcast_message(const char *message, int sender_fd, int channel, int sender_index) {
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

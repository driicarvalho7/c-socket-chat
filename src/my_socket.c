#include "my_socket.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

int my_socket() {
    int socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_fd < 0) {
        perror("Erro ao criar socket");
        exit(EXIT_FAILURE);
    }
    return socket_fd;
}

void my_bind(int socket_fd, struct sockaddr_in *address, int port) {
    address->sin_family = AF_INET;
    address->sin_addr.s_addr = INADDR_ANY;
    address->sin_port = htons(port);

    if (bind(socket_fd, (struct sockaddr *)address, sizeof(*address)) < 0) {
        perror("Erro no bind");
        close(socket_fd);
        exit(EXIT_FAILURE);
    }
}

void my_listen(int socket_fd, int max_clients) {
    if (listen(socket_fd, max_clients) < 0) {
        perror("Erro no listen");
        close(socket_fd);
        exit(EXIT_FAILURE);
    }
}

int my_accept(int socket_fd, struct sockaddr_in *client_addr) {
    socklen_t client_len = sizeof(*client_addr);
    int client_socket = accept(socket_fd, (struct sockaddr *)client_addr, &client_len);
    if (client_socket < 0) {
        perror("Erro ao aceitar conexão");
        return -1;
    }
    return client_socket;
}

void my_connect(int socket_fd, struct sockaddr_in *server_addr, const char *ip, int port) {
    server_addr->sin_family = AF_INET;
    server_addr->sin_port = htons(port);
    server_addr->sin_addr.s_addr = inet_addr(ip);

    if (connect(socket_fd, (struct sockaddr *)server_addr, sizeof(*server_addr)) < 0) {
        perror("Erro ao conectar ao servidor");
        close(socket_fd);
        exit(EXIT_FAILURE);
    }
}

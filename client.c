#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include "my_socket.h"

#define PORT 8881
#define BUFFER_SIZE 1024
#define NUM_CHANNELS 5

void *receive_messages(void *socket_desc) {
    int sock = *(int *)socket_desc;
    char buffer[BUFFER_SIZE];
    int bytes_read;

    while ((bytes_read = recv(sock, buffer, BUFFER_SIZE, 0)) > 0) {
        buffer[bytes_read] = '\0';
        printf("\r%s", buffer);
        printf("you: ");
        fflush(stdout);
    }

    return NULL;
}

int main() {
    int client_socket;
    struct sockaddr_in server_addr;
    char buffer[BUFFER_SIZE];
    char channel[BUFFER_SIZE];
    char username[50];
    pthread_t receive_thread;

    printf("###################################\n");
    printf("#                                 #\n");
    printf("#   BEM-VINDO AO C-SOCKET-CHAT!   #\n");
    printf("#                                 #\n");
    printf("###################################\n");
    printf("\n\n");
    printf("Digite seu nome de usuário: ");

    fgets(username, 50, stdin);
    username[strcspn(username, "\n")] = 0; // Remover o caractere de nova linha

    client_socket = my_socket();
    my_connect(client_socket, &server_addr, "127.0.0.1", PORT);

    // Enviar nome de usuário para o servidor
    if (send(client_socket, username, strlen(username), 0) < 0) {
        perror("Erro ao enviar nome de usuário");
        close(client_socket);
        exit(EXIT_FAILURE);
    }

    printf("\n\n");
    printf("###################################\n");
    printf("#                                 #\n");
    printf("#       ESCOLHA SEU CANAL!        #\n");
    printf("# 1. Geral                        #\n");
    printf("# 2. Jogos                        #\n");
    printf("# 3. Filmes                       #\n");
    printf("# 4. 4Chan                        #\n");
    printf("# 5. Relacionamentos              #\n");
    printf("#                                 #\n");
    printf("###################################\n");
    printf("\n\n");
    printf("Digite o canal: ");

    int canal_valido = 0;
    while (!canal_valido) {
        fgets(channel, BUFFER_SIZE, stdin);
        int canal = atoi(channel);
        if (canal >= 1 && canal <= NUM_CHANNELS) {
            canal_valido = 1;
        } else {
            printf("Canal inválido. Por favor, escolha um canal entre 1 e 5: ");
        }
    }

    if (send(client_socket, channel, strlen(channel), 0) < 0) {
        perror("Erro ao enviar escolha de canal");
        close(client_socket);
        exit(EXIT_FAILURE);
    }

    printf("Conectado ao servidor como %s no canal %d.\n", username, atoi(channel));

    if (pthread_create(&receive_thread, NULL, receive_messages, &client_socket) != 0) {
        perror("Erro ao criar thread de recebimento");
        close(client_socket);
        exit(EXIT_FAILURE);
    }

    while (1) {
        printf("you: ");
        if (fgets(buffer, BUFFER_SIZE, stdin) == NULL) {
            break;
        }
        if (send(client_socket, buffer, strlen(buffer), 0) < 0) {
            perror("Erro ao enviar mensagem");
            break;
        }
    }

    close(client_socket);
    return 0;
}

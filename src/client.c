#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <time.h>
#include <sys/ioctl.h>
#include "my_socket.h"
#include "globals.h"

// Função para simular delay na introdução
void delay(int milliseconds) {
    clock_t start_time = clock();
    while (clock() < start_time + milliseconds * CLOCKS_PER_SEC / 1000);
}

// Função para obter a largura atual do terminal
int get_terminal_width() {
    struct winsize w;
    ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
    return w.ws_col; // Retorna a largura em colunas
}

// Função para imprimir uma linha horizontal que ocupa toda a largura do terminal
void print_horizontal_line(char ch, int width, const char *color) {
    printf("%s", color);  // Aplicar a cor
    for (int i = 0; i < width; i++) {
        printf("%c", ch);
    }
    printf("\n" ANSI_COLOR_RESET);
}

// Função para centralizar o texto
void print_centered_text(const char *text, int width, const char *color) {
    int len = strlen(text);
    int padding = (width - len) / 2;
    printf("%s", color);  // Aplicar cor
    for (int i = 0; i < padding; i++) {
        printf(" ");
    }
    printf("%s\n", text);  // Corrigido: passar o texto como argumento para o printf
    printf(ANSI_COLOR_RESET);
}

// Função para limpar a tela
void clear_screen() {
    system("clear");  // No Windows, use system("cls");
}

// Função para exibir a introdução estilizada
void show_intro() {
    clear_screen();
    int width = get_terminal_width();
    print_horizontal_line('=', width, ANSI_COLOR_BLUE);
    print_centered_text("BEM-VINDO AO SISTEMA DE CHAT C-SOCKET", width, ANSI_COLOR_BLUE);
    print_horizontal_line('=', width, ANSI_COLOR_BLUE);
    delay(500); // Pequeno delay para simular uma animação
    printf("\nCarregando");
    for (int i = 0; i < 3; i++) {
        printf(".");
        fflush(stdout);
        delay(500); // Delay entre os pontos
    }
    delay(1000); // Adicionar um tempo extra para a tela ficar visível
}

// Função para exibir o menu de seleção de canal
int show_channel_menu() {
    clear_screen();
    int width = get_terminal_width();
    print_horizontal_line('=', width, ANSI_COLOR_YELLOW);
    print_centered_text("SELECIONE UM CANAL", width, ANSI_COLOR_YELLOW);
    print_horizontal_line('=', width, ANSI_COLOR_YELLOW);
    printf("\n 1. Geral\n 2. Jogos\n 3. Filmes\n 4. 4Chan\n 5. Relacionamentos\n");
    print_horizontal_line('=', width, ANSI_COLOR_YELLOW);
    printf(ANSI_COLOR_GREEN "\nDigite o número do canal que deseja entrar: " ANSI_COLOR_RESET);
    int channel;
    scanf("%d", &channel);

    // Verificar se o canal é válido
    while (channel < 1 || channel > NUM_CHANNELS) {
        printf(ANSI_COLOR_RED "Canal inválido! Escolha um número entre 1 e 5: " ANSI_COLOR_RESET);
        scanf("%d", &channel);
    }
    return channel;
}

// Função para exibir o chat dinamicamente
void show_chat_interface(char *username, int channel) {
    clear_screen();
    int width = get_terminal_width();
    print_horizontal_line('=', width, ANSI_COLOR_BLUE);
    char title[BUFFER_SIZE];
    sprintf(title, "Chat no Canal %d - Usuário: %s", channel, username);
    print_centered_text(title, width, ANSI_COLOR_BLUE);
    print_horizontal_line('=', width, ANSI_COLOR_BLUE);
}

// Função para receber mensagens do servidor e exibi-las ao usuário
void *receive_messages(void *socket_desc) {
    int sock = *(int *)socket_desc;
    char buffer[BUFFER_SIZE];
    int bytes_read;

    while ((bytes_read = recv(sock, buffer, BUFFER_SIZE, 0)) > 0) {
        buffer[bytes_read] = '\0';
        if (strstr(buffer, "you: ") != buffer) {
            printf(ANSI_COLOR_GREEN "[Mensagem] %s\n" ANSI_COLOR_RESET, buffer);
        } else if (strcmp(buffer, "username_in_use") == 0) {
            // Se o nome de usuário já estiver em uso, exibir erro e solicitar novamente
            printf(ANSI_COLOR_RED "Erro: Nome de usuário já em uso. Escolha outro.\n" ANSI_COLOR_RESET);
            printf(ANSI_COLOR_BLUE "\nDigite seu nome de usuário: " ANSI_COLOR_RESET);
            fgets(buffer, BUFFER_SIZE, stdin);
            buffer[strcspn(buffer, "\n")] = 0;
            send(sock, buffer, strlen(buffer), 0);
        }
        fflush(stdout);
    }

    if (bytes_read == 0) {
        printf(ANSI_COLOR_RED "\nServidor desconectado. Encerrando o cliente...\n" ANSI_COLOR_RESET);
    } else {
        perror("Erro ao receber mensagem do servidor");
    }

    close(sock);
    exit(EXIT_FAILURE);
}

int main() {
    int client_socket;
    struct sockaddr_in server_addr;
    char buffer[BUFFER_SIZE];
    char username[50];
    pthread_t receive_thread;

    // Exibir a introdução
    show_intro();

    // Solicitar o nome de usuário
    clear_screen();
    printf(ANSI_COLOR_BLUE "\nDigite seu nome de usuário: " ANSI_COLOR_RESET);
    fgets(username, 50, stdin);
    username[strcspn(username, "\n")] = 0;

    client_socket = my_socket();
    my_connect(client_socket, &server_addr, "127.0.0.1", PORT);

    // Enviar nome de usuário para o servidor
    if (send(client_socket, username, strlen(username), 0) < 0) {
        perror("Erro ao enviar nome de usuário");
        close(client_socket);
        exit(EXIT_FAILURE);
    }

    // Mostrar a tela de seleção de canal
    int channel = show_channel_menu();

    // Enviar escolha do canal para o servidor
    sprintf(buffer, "%d", channel);
    if (send(client_socket, buffer, strlen(buffer), 0) < 0) {
        perror("Erro ao enviar escolha de canal");
        close(client_socket);
        exit(EXIT_FAILURE);
    }

    // Mostrar a interface do chat
    show_chat_interface(username, channel);

    // Criar thread para receber mensagens do servidor
    if (pthread_create(&receive_thread, NULL, receive_messages, &client_socket) != 0) {
        perror("Erro ao criar thread de recebimento");
        close(client_socket);
        exit(EXIT_FAILURE);
    }

    // Loop principal para enviar mensagens
    while (1) {
        if (fgets(buffer, BUFFER_SIZE, stdin) == NULL) {
            break;
        }
        // Remover quebras de linha indesejadas
        buffer[strcspn(buffer, "\n")] = 0;

        // Verifica se o usuário digitou "SAIR"
        if (strcmp(buffer, "SAIR") == 0) {
            // Solicitar novo canal
            printf(ANSI_COLOR_YELLOW "Você escolheu sair. Selecione outro canal.\n" ANSI_COLOR_RESET);
            channel = show_channel_menu();
            sprintf(buffer, "change_channel %d", channel);  // Envia um comando especial ao servidor para mudar de canal
            if (send(client_socket, buffer, strlen(buffer), 0) < 0) {
                perror("Erro ao enviar escolha de canal");
                break;
            }
            show_chat_interface(username, channel);  // Atualiza a interface com o novo canal
            continue;  // Reinicia o loop de mensagens
        }

        // Enviar mensagem para o servidor
        if (send(client_socket, buffer, strlen(buffer), 0) < 0) {
            perror("Erro ao enviar mensagem");
            break;
        }
    }

    // Fechar socket do cliente
    close(client_socket);
    return 0;
}

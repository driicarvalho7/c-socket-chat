#ifndef GLOBALS_H
#define GLOBALS_H

#include <pthread.h>
#include "my_socket.h"

// Definições globais
#define PORT 8080                           // Porta de conexão entre cliente e servidor
#define BUFFER_SIZE 1024                    // Tamanho do buffer de mensagens
#define NUM_CHANNELS 5                      // Número de canais
#define MAX_CLIENTS 10                      // Número máximo de clientes
#define MAX_MESSAGES 100                    // Número máximo de mensagens em um canal
#define ANSI_COLOR_RED "\x1b[31m"           // Definição cor VERMELHA
#define ANSI_COLOR_BLUE "\x1b[34m"          // Definição cor AZUL
#define ANSI_COLOR_RESET "\x1b[0m"          // Definição cor PADRÃO
#define ANSI_COLOR_GREEN "\x1b[32m"         // Definição cor VERDE
#define ANSI_COLOR_YELLOW "\x1b[33m"        // Definição cor AMARELA

// Declaração das variáveis globais (extern)
extern int clients[MAX_CLIENTS];            // Array de sockets dos clientes
extern int client_channels[MAX_CLIENTS];    // Array que guarda o canal de cada cliente
extern int client_count;                    // Contador de clientes conectados
extern pthread_mutex_t clients_mutex;       // Mutex para proteger o acesso à lista de clientes

#endif

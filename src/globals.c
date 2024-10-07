#include "globals.h"

// Definição das variáveis globais
int clients[MAX_CLIENTS] = {0};                             // Array de sockets dos clientes
int client_channels[MAX_CLIENTS] = {0};                     // Array que guarda o canal de cada cliente
int client_count = 0;                                       // Contador de clientes conectados
pthread_mutex_t clients_mutex = PTHREAD_MUTEX_INITIALIZER;  // Mutex para proteger o acesso à lista de clientes

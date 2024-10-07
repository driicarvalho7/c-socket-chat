#ifndef LOGGER_H
#define LOGGER_H

#include <stdio.h>
#include <time.h>

// Função para iniciar o log
FILE* init_log();

// Função para escrever uma mensagem no log
void write_log(FILE *log_file, const char *message);

// Função para fechar o arquivo de log
void close_log(FILE *log_file);

#endif

#include "logger.h"
#include <stdlib.h>
#include <time.h>
#include <sys/stat.h>  // Para criar o diretório

FILE* init_log() {
    // Verificar e criar o diretório 'logs' se ele não existir
    struct stat st = {0};
    if (stat("logs", &st) == -1) {
        if (mkdir("logs", 0700) == -1) {
            perror("Erro ao criar diretório de logs");
            exit(EXIT_FAILURE);
        }
    }

    // Gerar timestamp para o nome do arquivo
    time_t now = time(NULL);
    struct tm *t = localtime(&now);
    
    // Gerar o nome do arquivo de log com timestamp
    char log_filename[100];
    snprintf(log_filename, sizeof(log_filename), "../logs/server_log_%04d-%02d-%02d_%02d-%02d-%02d.txt", 
             t->tm_year + 1900, t->tm_mon + 1, t->tm_mday, 
             t->tm_hour, t->tm_min, t->tm_sec);

    // Abrir o arquivo de log no diretório 'logs'
    FILE *log_file = fopen(log_filename, "a");
    if (!log_file) {
        perror("Erro ao abrir arquivo de log");
        exit(EXIT_FAILURE);
    }

    fprintf(log_file, "--- Início do Log: %s\n", ctime(&now));
    fflush(log_file);

    return log_file;
}

void write_log(FILE *log_file, const char *message) {
    time_t now = time(NULL);
    fprintf(log_file, "DATE: %sACTION: %s\n\n", ctime(&now), message);
    fflush(log_file);
}

void close_log(FILE *log_file) {
    if (log_file) {
        time_t now = time(NULL);
        fprintf(log_file, "--- Fim do Log: %s\n", ctime(&now));
        fclose(log_file);
    }
}

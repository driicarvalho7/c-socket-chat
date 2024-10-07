# Diretórios
SRC_DIR = src
INCLUDE_DIR = include
BUILD_DIR = build
BIN_DIR = bin

# Compilador e flags
CC = gcc
CFLAGS = -Wall -pthread -I$(INCLUDE_DIR)

# Alvos
SERVER_OBJ = $(BUILD_DIR)/server.o $(BUILD_DIR)/my_socket.o $(BUILD_DIR)/logger.o $(BUILD_DIR)/globals.o
CLIENT_OBJ = $(BUILD_DIR)/client.o $(BUILD_DIR)/my_socket.o $(BUILD_DIR)/globals.o

# Execução padrão (compila tudo)
all: directories server client

# Diretórios necessários
directories:
	mkdir -p $(BUILD_DIR) $(BIN_DIR)

# Compila o servidor
server: $(SERVER_OBJ)
	$(CC) $(CFLAGS) -o $(BIN_DIR)/server $(SERVER_OBJ)

# Compila o cliente
client: $(CLIENT_OBJ)
	$(CC) $(CFLAGS) -o $(BIN_DIR)/client $(CLIENT_OBJ)

# Regras de compilação
$(BUILD_DIR)/server.o: $(SRC_DIR)/server.c $(INCLUDE_DIR)/my_socket.h $(INCLUDE_DIR)/logger.h $(INCLUDE_DIR)/globals.h
	$(CC) $(CFLAGS) -c $(SRC_DIR)/server.c -o $@

$(BUILD_DIR)/client.o: $(SRC_DIR)/client.c $(INCLUDE_DIR)/my_socket.h $(INCLUDE_DIR)/globals.h
	$(CC) $(CFLAGS) -c $(SRC_DIR)/client.c -o $@

$(BUILD_DIR)/my_socket.o: $(SRC_DIR)/my_socket.c $(INCLUDE_DIR)/my_socket.h $(INCLUDE_DIR)/globals.h
	$(CC) $(CFLAGS) -c $(SRC_DIR)/my_socket.c -o $@

$(BUILD_DIR)/logger.o: $(SRC_DIR)/logger.c $(INCLUDE_DIR)/logger.h
	$(CC) $(CFLAGS) -c $(SRC_DIR)/logger.c -o $@

$(BUILD_DIR)/globals.o: $(SRC_DIR)/globals.c $(INCLUDE_DIR)/globals.h
	$(CC) $(CFLAGS) -c $(SRC_DIR)/globals.c -o $@

# Limpeza dos arquivos de build
clean:
	rm -rf $(BUILD_DIR) $(BIN_DIR)

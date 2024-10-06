CC = gcc
CFLAGS = -pthread

all: server client

server: server.c my_socket.c
	$(CC) $(CFLAGS) server.c my_socket.c -o server

client: client.c my_socket.c
	$(CC) $(CFLAGS) client.c my_socket.c -o client

clean:
	rm -f server client

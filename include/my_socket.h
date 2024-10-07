#ifndef MY_SOCKET_H
#define MY_SOCKET_H

#include <netinet/in.h>

int my_socket();
void my_bind(int socket_fd, struct sockaddr_in *address, int port);
void my_listen(int socket_fd, int max_clients);
int my_accept(int socket_fd, struct sockaddr_in *client_addr);
void my_connect(int socket_fd, struct sockaddr_in *server_addr, const char *ip, int port);

#endif

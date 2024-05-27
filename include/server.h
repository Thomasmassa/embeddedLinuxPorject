#ifndef SERVER_H
#define SERVER_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>
#include <arpa/inet.h>

int tcp_server_setup();
int tcp_server_listen(int server_fd);
void tcp_server_send(unsigned char *msg, int client_sockets[], int client_count);
void tcp_server_close();


#endif // TCP_SERVER_H

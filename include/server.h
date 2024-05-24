#ifndef SERVER_H
#define SERVER_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

int tcp_server_setup();
int handle_client(int new_socket, int queueID);
void tcp_server_listen(int queueID);
void tcp_server_close();

#endif // TCP_SERVER_H

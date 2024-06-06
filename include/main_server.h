#ifndef MAIN_SERVER_H
#define MAIN_SERVER_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <errno.h>
#include <unistd.h>
#include <signal.h>


void handle_signal(int signal);
void* Tcp_Send_Thread(void *arg);
void* Tcp_Listen_Thread(void *arg);
void handle_client(int socket, int queueID);
void* remove_socket(void *arg);



#endif  

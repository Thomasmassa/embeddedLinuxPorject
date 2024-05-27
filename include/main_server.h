#ifndef MAIN_SERVER_H
#define MAIN_SERVER_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <errno.h>
#include <unistd.h>

int isQueueEmpty();
int send_buffer(char* buffer);

#endif  

#ifndef QUEUE_H
#define QUEUE_H


#include <sys/ipc.h>   // Voor key_t, ftok()
#include <sys/msg.h>   // Voor msgget(), msgsnd(), msgrcv()
#include <stdio.h>     // Voor fprintf(), perror()
#include <string.h>    // Voor strerror()
#include <errno.h>     // Voor errno

int createQueue();
int openQueue();
int closeQueue(int queueID);


#endif  
struct message {
    long msg_type;
    char msg_text[100];
};
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <errno.h>
#include <unistd.h>
#include <sys/msg.h>
#include <signal.h>

#include "../include/terminal.h"
#include "../include/uart.h"
#include "../include/queue.h"

#define UARTALARM SIGALRM

pthread_cond_t cond = PTHREAD_COND_INITIALIZER;
pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
int uart_connected = 1;


pthread_t writethread, readthread;
int breakloop = 0;
char readbuffer[60];
int queueID1;


void closeAll() {
    breakloop = 21;
    pthread_cancel(writethread);
    pthread_cancel(readthread);
    pthread_cond_destroy(&cond);
    pthread_mutex_destroy(&lock);
    UART_close();
    exit(0);
}


void* UART_write_thread(void *arg) {    
    while (1) {

        pthread_mutex_lock(&lock);
        while (uart_connected == 0) {
            pthread_cond_wait(&cond, &lock);
        }//de thread wacht tot de conditie waar is en de lock wordt vrijgegeven
        pthread_mutex_unlock(&lock);
        
        struct message msg;
        if (msgrcv(queueID1, &msg, sizeof(msg), 1, 0) == -1) {
            fprintf(stderr, "Error from msgrcv: %s\n", strerror(errno));
            closeAll();
            continue;
        }

        UART_write(msg.msg_text);
    }
}


////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////


void* UART_read_thread(void *arg) {
    while (1) {

        pthread_mutex_lock(&lock);
        while (uart_connected == 0) {
            pthread_cond_wait(&cond, &lock);
        }//de thread wacht tot de conditie waar is en de lock wordt vrijgegeven
        pthread_mutex_unlock(&lock);    

        if(UART_read(readbuffer, sizeof(readbuffer)) != 0)
        {
            continue;
        }
        printf("Received message: %s", readbuffer);

        struct message msg;
        msg.msg_type = 2;
        strcpy(msg.msg_text, readbuffer);
        if (msgsnd(queueID1, &msg, sizeof(msg), 0) == -1) {
            fprintf(stderr, "Error from msgsnd: %s\n", strerror(errno));
            closeAll();
            continue;
        }
    }
}


////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////


void watchdog_timer() {
    if (UART_check_connection() != 0) {
        pthread_mutex_lock(&lock);//lock de mutex
        uart_connected = 0;//zet de conditie op 0 voor de andere threads
        pthread_mutex_unlock(&lock);//unlock de mutex

        perror("UART: LOST, retrying in 2sec\n");
        UART_close();
        if(UART_open() == 0)
        {
            perror("UART: RECONNECTED\n");
        }
        if (breakloop >= 21)
        {
            perror("UART connection lost, exiting program");    
            closeAll();
        }
        breakloop += 1;
    } else {
        pthread_mutex_lock(&lock);//lock de mutex
        uart_connected = 1;//zet de conditie op 1 voor de andere threads
        pthread_cond_broadcast(&cond);//broadcast de conditie naar de andere threads
        pthread_mutex_unlock(&lock);//unlock de mutex

        breakloop = 0;
    }

    alarm(2);
}


int startThreads() {
    if (pthread_create(&writethread, NULL, UART_write_thread, NULL) != 0) {
        perror("Failed to create UART write thread");
        return 1;
    }
    if (pthread_create(&readthread, NULL, UART_read_thread, NULL) != 0) {
        perror("Failed to create UART read thread");
        return 1;
    }
    return 0;
}


int main() {
    TerminalClear();

    queueID1 = openQueue(65);
    if (queueID1 == -1) {
        perror("Failed to open message queue");
        return 1;
    }printf("Message queue opened with ID: %d\n", queueID1);
    signal(UARTALARM, watchdog_timer);
    alarm(0);


    int try = 0;
    do {
        try++;
        uart_connected = UART_open();
        if (uart_connected != 0) {
            sleep(2);
        }
    } while (try < 21 && uart_connected != 0);
    if (uart_connected != 0) {
        perror("Failed to open UART, exiting program");
        closeQueue(queueID1);
        return 1;
    }

    if(startThreads() == 1) {
        perror("Failed to start threads");
        closeQueue(queueID1);
        UART_close();
        return 1;
    }printf("Threads started\n");

    alarm(8);

    pthread_join(writethread, NULL);
    pthread_join(readthread, NULL);
    return 0;
}

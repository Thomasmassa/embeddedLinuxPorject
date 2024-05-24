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


pthread_t writethread, readthread, watchdogthread;
int breakloop = 0;
char readbuffer[40];
int queueID;





int isQueueEmpty() {
    struct msqid_ds stats;
    if (msgctl(queueID, IPC_STAT, &stats) == -1) {
        perror("msgctl failed");
        return -1;
    }
    return stats.msg_qnum == 0;
}


////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////


void* UART_write_thread(void *arg) {    
    while (1) {

        pthread_mutex_lock(&lock);
        while (uart_connected == 0) {
            pthread_cond_wait(&cond, &lock);
        }//de thread wacht tot de conditie waar is en de lock wordt vrijgegeven
        pthread_mutex_unlock(&lock);

        if (isQueueEmpty(queueID)) {
            continue;
        }
        
        struct message msg;
        if (msgrcv(queueID, &msg, sizeof(msg), 1, 0) == -1) {
            fprintf(stderr, "Error from msgrcv: %s\n", strerror(errno));
            continue;
        }

        UART_write(msg.msg_text);

        sleep(0.2);
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

        sleep(0.2);
        if(UART_read(readbuffer, sizeof(readbuffer)) != 0)
        {
            continue;
        }
        printf("Received message: %s\n", readbuffer);
    }
}


////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////


void* watchdog_timer(void* arg) {
    while (1) {
        if (UART_check_connection() != 0) {
            pthread_mutex_lock(&lock);//lock de mutex
            uart_connected = 0;//zet de conditie op 0 voor de andere threads
            pthread_mutex_unlock(&lock);//unlock de mutex

            perror("UART connection lost, retrying...\n");
            UART_close();
            UART_open();
            if (breakloop > 10)
            {
                perror("UART connection lost, exiting program");
                closeQueue(queueID);
                exit(1);
            }
            breakloop += 1;
        } else {
            pthread_mutex_lock(&lock);//lock de mutex
            uart_connected = 1;//zet de conditie op 1 voor de andere threads
            pthread_cond_broadcast(&cond);//broadcast de conditie naar de andere threads
            pthread_mutex_unlock(&lock);//unlock de mutex

            breakloop = 0;
            printf("UART connection is still alive\n");
        }

        sleep(8);
    }
}


////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////


int main() {
    TerminalClear();
    queueID = createQueue();
    if (queueID == -1) {
        perror("Failed to create message queue");
        return 1;
    }
    else
    {
        
        int try = 0;
        do {
            try++;
            uart_connected = UART_open();
            if (uart_connected != 0) {
                sleep(4);
            }
        } while (try < 11 && uart_connected != 0);
        if (uart_connected != 0) {
            perror("Failed to open UART, exiting program");
            closeQueue(queueID);
            return 1;
        }


        printf("Message queue created with ID: %d\n", queueID);

        if (pthread_create(&watchdogthread, NULL, watchdog_timer, NULL) != 0) {
            perror("Failed to create watchdog timer thread");
            return 1;
        }   
        if (pthread_create(&writethread, NULL, UART_write_thread, NULL) != 0) {
            perror("Failed to create UART write thread");
            return 1;
        }
        if (pthread_create(&readthread, NULL, UART_read_thread, NULL) != 0) {
            perror("Failed to create UART read thread");
            return 1;
        }

        pthread_join(writethread, NULL);
        pthread_join(readthread, NULL);
        pthread_join(watchdogthread, NULL);
    
    }    

    closeQueue(queueID);
    return 0;
}


//test
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

pthread_t writethread, readthread;
int breakloop = 0;
char readbuffer[64];
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

        if (isQueueEmpty(queueID)) {
            usleep(2000000);//0.2 seconde
            continue;
        }
        
        struct message msg;
        if (msgrcv(queueID, &msg, sizeof(msg), 1, 0) == -1) {
            fprintf(stderr, "Error from msgrcv: %s\n", strerror(errno));
            continue;
        }

        printf("Received message: %s\n", msg.msg_text);
        UART_write(msg.msg_text);

        usleep(2000000);//0.2 seconde
    }
}


////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////


void* UART_read_thread(void *arg) {
    while (1) {
        usleep(2000000);//0.1 seconde
        if(UART_read(readbuffer, sizeof(readbuffer)) != 0)
        {
            perror("Failed to read from UART");
            continue;
        }
        printf("Received message: %s\n", readbuffer);
    }
}


////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////


void watchdog_timer(int sig) {
    if (sig == UARTALARM) {  
        if (UART_check_connection() != 0) {
            perror("UART connection lost");
            UART_close();
            UART_open();
            alarm(3);
            if (breakloop > 10)
            {
                perror("UART connection lost, exiting program");
                closeQueue(queueID);
                exit(1);//exit the program
            }
            breakloop += 1;
        }else
        {
            breakloop = 0;
            alarm(5);  // Reset the timer
            printf("UART connection is still active\n");
        }
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
        signal(UARTALARM, watchdog_timer);
        alarm(5);

        printf("Message queue created with ID: %d\n", queueID);

        int* queueIDPtr = malloc(sizeof(int));
        *queueIDPtr = queueID;

        int uartOpenResult;
        int attempts = 0;
        do {
            uartOpenResult = UART_open();
            if (uartOpenResult != 0) {
                perror("Failed to open UART, retrying...");
                sleep(1);  // Wait for 1 second before retrying
            }
            attempts++;
        } while (uartOpenResult != 0 && attempts < 10);

        if (uartOpenResult != 0) {
            perror("Failed to open UART after 10 attempts");
            return 1;
        }
        else
        {
            printf("UART opened\n");
              if (pthread_create(&writethread, NULL, UART_write_thread, queueIDPtr) != 0) {
            perror("Failed to create UART write thread");
            return 1;
            }
            if (pthread_create(&readthread, NULL, UART_read_thread, NULL) != 0) {
                perror("Failed to create UART read thread");
                return 1;
            }

            pthread_join(writethread, NULL);
            pthread_join(readthread, NULL);
        }
    }    

    closeQueue(queueID);
    return 0;
}
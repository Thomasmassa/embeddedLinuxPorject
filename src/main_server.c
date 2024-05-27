#include "../include/main_server.h"
#include "../include/terminal.h"
#include "../include/server.h"
#include "../include/queue.h"

pthread_t send_thread, listen_thread;

int client_sockets[10];
int client_count = 0;


int isQueueEmpty(int queueID2) {
    struct msqid_ds stats;
    if (msgctl(queueID2, IPC_STAT, &stats) == -1) {
        perror("msgctl failed");
        return -1;
    }
    return stats.msg_qnum == 0;
}


////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////


void* Tcp_Send_Thread(void *arg) {
    int queueID2 = *((int*)arg);
    while (1) {
        if (isQueueEmpty(queueID2))
        {
            continue;
        }
        struct message msg;
        if (msgrcv(queueID2, &msg, sizeof(msg), 0, 0) == -1) {
            fprintf(stderr, "Error from msgrcv: %s\n", strerror(errno));
            return NULL;
        }
        printf("Message received from queue: %s", msg.msg_text);
    
        tcp_server_send(msg.msg_text, client_sockets, client_count);
    }
}


////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////


void remove_socket(int socket_to_remove) {
    int i;
    for (i = 0; i < client_count; i++) {
        if (client_sockets[i] == socket_to_remove) {
            break;
        }
    }

    if (i < client_count) {
        for (int j = i; j < client_count - 1; j++) {
            client_sockets[j] = client_sockets[j + 1];
        }
        (client_count)--;
    }
}


////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////


void handle_client(int socket, int queueID) 
{
    char buffer[1024];
    int read_size;

    while (1) {
        memset(buffer, 0, sizeof(buffer));  // Initialiseer de buffer met nullen
        read_size = read(socket, buffer, sizeof(buffer) - 1);
        if (read_size <= 0)
            break;

        //check of de buffer leeg is
        //als de buffer leeg is dan 
        //gaat de while loop weer opnieuw
        char *bufferP = buffer;
        while (*bufferP != '\0') {
            if (!isspace((unsigned char)*bufferP)) {
                break;
            }
            bufferP++;
        }
        if (*bufferP == '\0') {
            continue;
        }

        buffer[read_size] = '\0';
        printf("Received message: %s\n", buffer);

        struct message msg;
        msg.msg_type = 1;
        strcpy(msg.msg_text, buffer);
        if (msgsnd(queueID, &msg, sizeof(msg), 0) == -1) {
            fprintf(stderr, "Error from msgsnd: %s\n", strerror(errno));
            continue;
        }
        printf("Message sent to queue %s\n", buffer);
    }

    remove_socket(socket);
    printf("Connection closed\n");
    close(socket);
}


void* Tcp_Listen_Thread(void *arg) {
    int queueID1 = *((int*)arg);
    while (1) {
        int new_socket = tcp_server_listen(queueID1);

        if (new_socket == -1) {
            fprintf(stderr, "Error from tcp_server_listen\n");
            continue;
        }

        client_sockets[client_count] = new_socket;
        client_count++;

        pid_t pid = fork();
        if (pid < 0) {
            perror("Failed to fork");
            continue;
        }
        if (pid == 0) {
            //kindproces
            handle_client(new_socket, queueID1);
            exit(0);
        } else {
            //ouderproces
            close(new_socket);
        }   
    }
}



int main() {
    TerminalClear();
    printf("Start Program\n");

    // QUEUE 1
    int queueID1 = createQueue(65);//key 65 queue server -> uart
    if (queueID1 == -1) {
        perror("Failed to create message queue");
        return 1;
    }
    printf("Message queue created with ID: %d\n", queueID1);
    // QUEUE 1

    if (tcp_server_setup() == -1) {
        perror("Failed to setup TCP server");
        return 1;
    }//Server opzetten

    // QUEUE 2
    int queueID2 = openQueue(66);//key 66 queue uart -> server
    if (queueID2 == -1) {
        perror("Failed to open message queue");
        return 1;
    }
    printf("Message queue opened with ID: %d\n", queueID2);
    // QUEUE 2

    // THREADS 
    int *pQueueID1 = malloc(sizeof(int));
    if (pQueueID1 == NULL) {
        perror("Failed to allocate memory");
        return 1;
    }
    *pQueueID1 = queueID1;
    if (pthread_create(&listen_thread, NULL, Tcp_Listen_Thread, pQueueID1) != 0) {
        perror("Failed to create listen thread");
        return 1;
    }
    // //////////////////////////////////////////
    int *pQueueID2 = malloc(sizeof(int));
    if (pQueueID2 == NULL) {
        perror("Failed to allocate memory");
        return 1;
    }
    *pQueueID2 = queueID2;
    if (pthread_create(&send_thread, NULL, Tcp_Send_Thread, pQueueID2) != 0) {
        perror("Failed to create send thread");
        return 1;
    }
    // THREADS


    pthread_join(listen_thread, NULL);
    pthread_join(send_thread, NULL);


    closeQueue(queueID1);
    closeQueue(queueID2);
    tcp_server_close();
    return 0;
}
#include "../include/main_server.h"
#include "../include/terminal.h"
#include "../include/server.h"
#include "../include/queue.h"


pthread_t send_thread, listen_thread, remove_socket_thread;

int client_sockets[10];
int client_count = 0;

int queueID1;

#include <sys/msg.h>

#define MAX_QUEUE_SIZE 100



void* Tcp_Send_Thread(void *arg) {
    while (1) {
        struct message msg;
        if (msgrcv(queueID1, &msg, sizeof(msg), 2, 0) == -1) {
            fprintf(stderr, "Error from msgrcv: %s\n", strerror(errno));
            exit(0);
        }
        printf("Message received from queue: %s", msg.msg_text);
    
        tcp_server_send(msg.msg_text, client_sockets, client_count);
    }
    return NULL;
}


////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////


void* remove_socket(void *arg) {
    int socket_to_remove;
    while(1)
    {
        usleep(100000);
        struct message msg;
        if (msgrcv(queueID1, &msg, sizeof(msg), 3, 0) == -1) {
            fprintf(stderr, "Error from msgrcv: %s\n", strerror(errno));
            exit(0);
        }
        socket_to_remove = atoi(msg.msg_text);

        int i;
        printf("Removing socket %d\n", socket_to_remove);
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
            printf("Socket removed, socket count: %d\n", client_count);
        }
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
        read_size = read(socket, buffer, sizeof(buffer) - 1);//lees de input van de client
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

    printf("Connection closed\n");
    close(socket);

    struct message msg;
    msg.msg_type = 3;
    sprintf(msg.msg_text, "%d", socket);
    if (msgsnd(queueID, &msg, sizeof(msg), 0) == -1) {
        fprintf(stderr, "Error from msgsnd: %s\n", strerror(errno));
    }
}


void* Tcp_Listen_Thread(void *arg) {
    while (1) {
        int new_socket = tcp_server_listen(queueID1);

        if (new_socket == -1) {
            fprintf(stderr, "Error from tcp_server_listen\n");
            continue;
        }

        client_sockets[client_count] = new_socket;
        client_count++;

        printf("socket count: %d)\n", client_count);

        pid_t pid = fork();
        if (pid < 0) {
            perror("Failed to fork");
            continue;
        }
        if (pid == 0) {
            //kindproces
            handle_client(new_socket, queueID1);
            exit(0);
        } 
    }
}

void handle_sigint(int sig) {
    closeQueue(queueID1);
    tcp_server_close();
    exit(0);
}

int main() {
    TerminalClear();
    signal(SIGINT, handle_sigint);
    closeQueue(65);

    queueID1 = createQueue(65);
    if (queueID1 == -1) {
        perror("Failed to create message queue");
        return 1;
    }printf("Message queue created with ID: %d\n", queueID1);

    if (tcp_server_setup() == -1) {
        perror("Failed to setup TCP server");
        return 1;
    }//Server opzetten

    // THREADS 
    ///////////////////////////////////////////////
    if (pthread_create(&listen_thread, NULL, Tcp_Listen_Thread, NULL) != 0) {
        perror("Failed to create listen thread");
        return 1;
    }

    if (pthread_create(&send_thread, NULL, Tcp_Send_Thread, NULL) != 0) {
        perror("Failed to create send thread");
        return 1;
    }

    if (pthread_create(&remove_socket_thread, NULL, remove_socket, NULL) != 0) {
        perror("Failed to create remove socket thread");
        return 1;
    }
    // THREADS

    pthread_join(send_thread, NULL);
    printf("Send thread joined\n");

    pthread_join(remove_socket_thread, NULL);
    pthread_join(listen_thread, NULL);

    closeQueue(queueID1);
    tcp_server_close();
    return 0;
}
#include "../include/server.h"
#include "../include/main_server.h"
#include "../include/queue.h"

#define PORT 8080

int server_fd, new_socket;
struct sockaddr_in address;
int addrlen = sizeof(address);

int tcp_server_setup() {
    int opt = 1;

    printf("Starting TCP server setup\n");

    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }//check of de socket is aangemaakt en of dit gelukt is

    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) {
        perror("setsockopt");
        close(server_fd);
        exit(EXIT_FAILURE);
    }//check of de socket opties zijn aangepast en of dit gelukt is

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);
    printf("Binding to port %d\n", PORT);

    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("bind failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    } //check of de socket is gebonden aan het adres en of dit gelukt is

    if (listen(server_fd, 3) < 0) {
        perror("listen");
        close(server_fd);
        exit(EXIT_FAILURE);
    } //check of de socket luistert en of dit gelukt is

    return 0;
}


int handle_client(int new_socket, int queueID) {
    char buffer[1024];
    int read_size;

    while ((read_size = read(new_socket, buffer, sizeof(buffer) - 1)) > 0) {
        buffer[read_size] = '\0';
        printf("Received message: %s\n", buffer);
        struct message msg;
        msg.msg_type = 1;
        strcpy(msg.msg_text, buffer);
        if (msgsnd(queueID, &msg, sizeof(msg), 0) == -1) {
            fprintf(stderr, "Error from msgsnd: %s\n", strerror(errno));
            return -1;
        }
        printf("Message sent to queue\n");
    }

    printf("Connection closed\n");
    close(new_socket);
}

void tcp_server_listen(int queueID) {
    while (1) {
        int new_socket;
        if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen)) < 0) {
            perror("accept");
            return;
        }
        printf("Connection accepted\n");

        pid_t pid = fork();
        if (pid < 0) {
            perror("Failed to fork");
            return;
        }

        if (pid == 0) {
            //kindproces
            handle_client(new_socket, queueID);
            exit(0);
        } else {
            //ouderproces
            close(new_socket);
        }
    }
}


void tcp_server_close() {
    close(server_fd);
}
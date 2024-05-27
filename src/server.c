#include "../include/server.h"
#include "../include/main_server.h"
#include "../include/queue.h"

#define PORT 8080

int server_fd;
struct sockaddr_in address;
int addrlen = sizeof(address);

int tcp_server_setup() 
{
    int opt = 1;

    printf("Starting TCP server setup\n");

    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("socket failed");
        return -1;
    }//check of de socket is aangemaakt en of dit gelukt is

    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) {
        perror("setsockopt");
        close(server_fd);
        return -1;
    }//check of de socket opties zijn aangepast en of dit gelukt is

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);
    printf("Binding to port %d\n", PORT);

    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("bind failed");
        close(server_fd);
        return -1;
    } //check of de socket is gebonden aan het adres en of dit gelukt is

    if (listen(server_fd, 3) < 0) {
        perror("listen");
        close(server_fd);
        return -1;
    } //check of de socket luistert en of dit gelukt is

    printf("TCP server setup complete\n");
    return 0;
}


////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////


int tcp_server_listen(int queueID) 
{
    int new_socket;
    if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen)) < 0) {
        perror("accept");
        return -1;
    }
    printf("Connection accepted (socket: %d)\n", new_socket);
    return new_socket;
}


////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////


void tcp_server_send(unsigned char *msg, int client_sockets[], int client_count) 
{
    for (int i = 0; i < client_count; i++) {
        printf("sending message to (%d): %s",client_sockets[i] , msg);
        write(client_sockets[i], msg, strlen(msg));
        write(client_sockets[i], "\n", 1);
    }
}


////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////


void tcp_server_close() {
    close(server_fd);
}
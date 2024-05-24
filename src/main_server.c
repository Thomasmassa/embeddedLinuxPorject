#include "../include/main_server.h"
#include "../include/terminal.h"
#include "../include/server.h"
#include "../include/queue.h"

int main() {
    TerminalClear();
    printf("Start Program\n");

    int queueID = createQueue();
    if (queueID == -1) {
        perror("Failed to create message queue");
        return 1;
    }
    printf("Message queue created with ID: %d\n", queueID);

    if (tcp_server_setup() != 0) {
        perror("Failed to setup TCP server");
        return 1;
    }
    printf("TCP server setup complete\n");

    tcp_server_listen(queueID);
    closeQueue(queueID);
    tcp_server_close();
    return 0;
}
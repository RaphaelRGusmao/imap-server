/**
    IMAP connect module
    Created by:
    Pedro Pereira, 9778794
    Raphael Gusmão, 9778561
 */

#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include "error_handler.h"

#define LIMIT_QUEUE 10

typedef struct {
    int socket;
    int address_lenght;
    struct sockaddr client_address;
} connection;

int TCP_create_socket(void) {
    add_to_stack("connect->create_tcp_socket");
    int socket_id = socket(PF_INET, SOCK_STREAM, 0);
    if (socket_id == -1) {
        die_with_msg("sock_id returned -1: Failure to bind socket");
    }
    pop_stack();
    return socket_id;
}

void TCP_close_socket(int socket_id) {
    add_to_stack("connect->close");
    int result = close(socket_id);
    if (result == -1) {
        die_with_msg("Failure to close socket");
    }
    pop_stack();
}

void TCP_SERVER_start_listen(int socket_id) {
    add_to_stack("connect[server]->start_listen");
    int result = listen(socket_id, LIMIT_QUEUE);
    if (result == -1) {
        die_with_msg("Failure to start listening in socket %d", socket_id);
    }
    pop_stack();
}

connection TCP_SERVER_accept_connection(int socket_id) {
    add_to_stack("connect[server]->accept");
    connection new_connection;
    int address_lenght = sizeof(new_connection.client_address)
    int new_socket = accept(socket_id, &(new_connection.client_address), &address_lenght);
    new_connection.address_lenght = address_lenght;
    new_connection.socket = new_socket;
    return new_connection;
}

void TCP_CLIENT_start_connect(int socket_id, struct sockaddr_in* address) {
    add_to_stack("connect[client]->connect")
    memset(address, 0, sizeof(address));
    int result = connect(socket_id, (struct sockaddr *) address, sizeof(address));
    if (result == -1) {
        die_with_msg("Failure to connect at socket %d", socket_id);
    }
    pop_stack();
}

int main() {
    int sock = create_tcp_socket();
    printf("new sock: %d\n", sock);
    close_tcp_socket(sock);
    return 0;
}

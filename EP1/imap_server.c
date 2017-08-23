#include "tcp_handler.h"
#include "error_handler.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>

#define true 1
#define false 0
#define MAXMSG 4095

void clean_vector(char *vector, int size) {
    for (int i = 0; i < size; i++) {
        vector[i] = '\0';
    }
}

int main(int argc, char **argv) {
    int listen_socket;

    if (argc != 2) {
        fprintf(stderr, "Usage: %s <port>\n", argv[0]);
        exit(1);
    }

    listen_socket = TCP_SERVER_create_socket(atoi(argv[1]));

    printf("Server running at port %s.\n", argv[1]);
    while (true) {
        int connection_socket = TCP_SERVER_accept_connection(listen_socket);
        if (connection_socket < 0) {
            die_with_msg("connection_id received -1: failed to accept connection");
        }

        pid_t childpid = fork();
        if (childpid == 0) {
            // CHILD PROCESS
            // closes passive listening socket
            close(listen_socket);
            // Buffer that will be fillled with messages
            char line_buffer[MAXMSG + 1];
            ssize_t line_size;

            // BEGIN EP1
            /* Client sends a message */
            while ((line_size = read(connection_socket, line_buffer, MAXMSG)) > 0) {
                write(connection_socket, line_buffer, strlen(line_buffer));
                clean_vector(line_buffer, MAXMSG + 1);
            }
            exit(0);
        }
        else {
            // PARENT PROCESS
            // Closes active connection and returns to listening
            close(connection_socket);
        }
    }
    return 0;
}

#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include "tcp_handler.h"
#include "error_handler.h"

int TCP_SERVER_create_socket(uint16_t socket_port) {
    add_to_stack("TCP[server]->create_socket");
    /*
    struct sockaddr_in:
        unsigned short sin_family
        unsigned short sin_port
        struct in_addr sin_addr:
            unsigned long s_addr
        unsigned short sin_zero[8]
     */
    struct sockaddr_in address;
    int sock;

    sock = socket(PF_INET, SOCK_STREAM, 0);
    if (sock == -1) {
        die_with_msg("Unable to create socket");
    }

    /*
    By convention, we always set the entire structure to 0 before filling
    it in, not just the sin_zero member.
     */
    memset(&address, 0, sizeof(address));

    address.sin_family = AF_INET;
    // network is big-endian: converts if necessary
    address.sin_port = htons(socket_port);
    address.sin_addr.s_addr = htonl(INADDR_ANY); // Accepts any connection

    int result = bind(sock, (struct sockaddr *) &address, sizeof(address));
    if (result < 0) {
        die_with_msg("Unable to bind socket to address. Socket = %d", sock);
    }

    result = listen(sock, 1);
    if (result < 0) {
      die_with_msg("Unable to listen in socket %d", sock);
    }
    pop_stack();
    return sock;
}

int TCP_SERVER_accept_connection(int socket_id) {
    return accept(socket_id, (struct sockaddr *) NULL, NULL);
}

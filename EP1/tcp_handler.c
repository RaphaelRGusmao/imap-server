/******************************************************************************
 *                               IME-USP (2017)                               *
 *       MAC0352 - Redes de Computadores e Sistemas Distribuidos - EP1        *
 *                                                                            *
 *                                TCP Handler                                 *
 *                                                                            *
 *                      Pedro Pereira     - NUSP 9778794                      *
 *                      Raphael R. Gusmao - NUSP 9778561                      *
 ******************************************************************************/

#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "error_handler.h"
#include "tcp_handler.h"

int TCP_SERVER_create_socket (uint16_t socket_port)
{
    add_to_stack("TCP[server] -> create_socket()");

    struct sockaddr_in address;

    int new_socket = socket(PF_INET, SOCK_STREAM, 0);
    if (new_socket == -1) {
        die_with_msg("Unable to create socket");
    }

    memset(&address, 0, sizeof(address));
    address.sin_family = AF_INET;
    address.sin_port = htons(socket_port);
    address.sin_addr.s_addr = htonl(INADDR_ANY);

    if (bind(new_socket, (struct sockaddr *)&address, sizeof(address)) == -1) {
        die_with_msg("Unable to bind socket to address. Socket = %d", new_socket);
    }

    if (listen(new_socket, 1) == -1) {
      die_with_msg("Unable to listen in socket %d", new_socket);
    }

    pop_stack();
    return new_socket;
}

int TCP_SERVER_accept_connection (int socket_id)
{
    return accept(socket_id, (struct sockaddr *) NULL, NULL);
}

/******************************************************************************/

#ifndef __TCP_HANDLE_H__
#define __TCP_HANDLE_H__

#include <stdint.h>
#define LIMIT_CONNECTIONS 10

/**
 * Handles client connection
 * @param client_socket
 */
void TCP_SERVER_handle_client(int client_socket);

int TCP_SERVER_create_socket(uint16_t socket_port);

int TCP_SERVER_accept_connection(int socket_id);

#endif

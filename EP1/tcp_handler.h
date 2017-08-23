#ifndef __TCP_HANDLE_H__
#define __TCP_HANDLE_H__

#include <stdint.h>
#define LIMIT_CONNECTIONS 10


int TCP_SERVER_create_socket(uint16_t socket_port);

int TCP_SERVER_accept_connection(int socket_id);

#endif

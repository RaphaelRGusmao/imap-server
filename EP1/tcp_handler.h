/******************************************************************************
 *                               IME-USP (2017)                               *
 *       MAC0352 - Redes de Computadores e Sistemas Distribuidos - EP1        *
 *                                                                            *
 *                                TCP Handler                                 *
 *                                                                            *
 *                      Pedro Pereira     - NUSP 9778794                      *
 *                      Raphael R. Gusmao - NUSP 9778561                      *
 ******************************************************************************/

#ifndef __TCP_HANDLE_H__
#define __TCP_HANDLE_H__

#include <stdint.h>
#define LIMIT_CONNECTIONS 10

int TCP_SERVER_create_socket (uint16_t socket_port);

int TCP_SERVER_accept_connection (int socket_id);

#endif

/******************************************************************************/

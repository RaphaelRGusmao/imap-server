/******************************************************************************
 *                               IME-USP (2017)                               *
 *       MAC0352 - Redes de Computadores e Sistemas Distribuidos - EP1        *
 *                                                                            *
 *                                IMAP server                                 *
 *                                                                            *
 *                      Pedro Pereira     - NUSP 9778794                      *
 *                      Raphael R. Gusmao - NUSP 9778561                      *
 ******************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "_aux.h"
#include "error_handler.h"
#include "imap_commands.h"
#include "tcp_handler.h"

#define MAXN 4096

char *IMAP_execute (const char *line)
{
    add_to_stack("IMAP -> execute()");
    char *ret = emalloc(MAXN * sizeof(char));
    string_vector split_line = split(line, " \n\r\t");

    if (split_line.size == 0) {
        ret = "BAD Empty command line\n";
    } else if (strcmp(upper_case(split_line.data[0]), "NOOP") == 0) {
        if (split_line.size == 1)
            ret = IMAP_noop();
        else
            ret = "BAD Invalid arguments\n";
    } else if (strcmp(upper_case(split_line.data[0]), "LOGOUT") == 0) {
        if (split_line.size == 1)
            ret = IMAP_logout();
        else
            ret = "BAD Invalid arguments\n";
    } else if (strcmp(upper_case(split_line.data[0]), "STARTTLS") == 0) {
        if (split_line.size == 1)
            ret = IMAP_starttls();
        else
            ret = "BAD Invalid arguments\n";
    } else if (strcmp(upper_case(split_line.data[0]), "AUTHENTICATE") == 0) {
        if (split_line.size == 2)
            ret = IMAP_authenticate(split_line.data[1]);
        else
            ret = "BAD Invalid arguments\n";
    } else if (strcmp(upper_case(split_line.data[0]), "LOGIN") == 0) {
        if (split_line.size == 3)
            ret = IMAP_login(split_line.data[1], split_line.data[2]);
        else
            ret = "BAD Invalid arguments\n";
    } else if (strcmp(upper_case(split_line.data[0]), "CREATE") == 0) {
        if (split_line.size == 2)
            ret = IMAP_create(split_line.data[1]);
        else
            ret = "BAD Invalid arguments\n";
    } else if (strcmp(upper_case(split_line.data[0]), "DELETE") == 0) {
        if (split_line.size == 2)
            ret = IMAP_delete(split_line.data[1]);
        else
            ret = "BAD Invalid arguments\n";
    } else if (strcmp(upper_case(split_line.data[0]), "RENAME") == 0) {
        if (split_line.size == 3)
            ret = IMAP_rename(split_line.data[1], split_line.data[2]);
        else
            ret = "BAD Invalid arguments\n";
    } else if (strcmp(upper_case(split_line.data[0]), "LIST") == 0) {
        if (split_line.size == 3)
            ret = IMAP_list(split_line.data[1], split_line.data[2]);
        else
            ret = "BAD Invalid arguments\n";
    } else if (strcmp(upper_case(split_line.data[0]), "EXPUNGE") == 0) {
        if (split_line.size == 1)
            ret = IMAP_expunge();
        else
            ret = "BAD Invalid arguments\n";
    } else {
        snprintf(ret, MAXN, "BAD No such command as \"%s\"\n", split_line.data[0]);
    }

    free_vector(split_line);
    pop_stack();
    return ret;
}

int main (int argc, char **argv)
{
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <port>\n", argv[0]);
        exit(1);
    }

    int listen_socket = TCP_SERVER_create_socket(atoi(argv[1]));
    printf("Server running at port %s.\n", argv[1]);

    for (;;) {
        int connection_socket = TCP_SERVER_accept_connection(listen_socket);
        if (connection_socket == -1) {
            die_with_msg("Failed to accept connection");
        }

        if (fork() == 0) {// Child Process
            close(listen_socket);
            char line[MAXN];
            ssize_t line_size;

            // Client sends a command
            while ((line_size = read(connection_socket, line, MAXN-1)) > 0) {
                line[line_size] = 0;
                char *result = IMAP_execute(line);
                write(connection_socket, result, strlen(result));
            }

            exit(0);
        } else {// Parent Process
            close(connection_socket);
        }
    }
    return 0;
}

/******************************************************************************/

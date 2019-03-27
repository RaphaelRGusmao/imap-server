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

#define MAX_LENGTH 4096

int is_online = 1;

/******************************************************************************/
char *IMAP_execute (const char *line)
{
    add_to_stack("imap_server -> IMAP_execute()");
    char *ret = emalloc(MAX_LENGTH * sizeof(char));
    string_vector split_line = split(line, " \n\r\t");
    if (split_line.size > 0) {
        char *tag = split_line.data[0];
        if (split_line.size == 1) {
            ret = format("%s %s", tag, "BAD Empty command line\n");
        } else {
            char *command = split_line.data[1];
            command = upper_case(command);
            /******************************************************************/
            if (!strcmp(command, "CAPABILITY")) {
                if (split_line.size == 2) {// 0 arguments
                    ret = IMAP_capability(tag);
                } else {
                    ret = format("%s %s", tag, "BAD Invalid arguments\n");
                }
            /******************************************************************/
            } else if (!strcmp(command, "NOOP")) {
                if (split_line.size == 2) {// 0 arguments
                    ret = IMAP_noop();
                } else {
                    ret = "BAD Invalid arguments\n";
                }
                ret = format("%s %s", tag, ret);
            /******************************************************************/
            } else if (!strcmp(command, "LOGOUT")) {
                if (split_line.size == 2) {// 0 argument
                    ret = IMAP_logout(tag);
                    is_online = 0;
                } else {
                    ret = format("%s %s", tag, "BAD Invalid arguments\n");
                }
            /******************************************************************/
            } else if (!strcmp(command, "LOGIN")) {
                if (split_line.size == 4 ) {// 2 arguments
                    ret = IMAP_login(split_line.data[2], split_line.data[3]);
                } else {
                    ret = "BAD Invalid arguments\n";
                }
                ret = format("%s %s", tag, ret);
            /******************************************************************/
            } else if (!strcmp(command, "SELECT")) {
                if (split_line.size == 3 ) {// 1 argument (not used)
                    ret = IMAP_select(tag);
                } else {
                    ret = format("%s %s", tag, "BAD Invalid arguments\n");
                }
            /******************************************************************/
            } else if (!strcmp(command, "LIST")) {
                if (split_line.size == 4) {// 2 arguments (not used)
                    ret = IMAP_list(tag);
                } else {
                    ret = format("%s %s", tag, "BAD Invalid arguments\n");
                }
            /******************************************************************/
            } else if (!strcmp(command, "LSUB")) {
                if (split_line.size == 4) {// 2 arguments (not used)
                    ret = IMAP_lsub();
                } else {
                    ret = "BAD Invalid arguments\n";
                }
                ret = format("%s %s", tag, ret);
            /******************************************************************/
            } else if (!strcmp(command, "CLOSE")) {
                if (split_line.size >= 2) {// 0 arguments
                    ret = IMAP_close();
                } else {
                    ret = "BAD Invalid arguments\n";
                }
                ret = format("%s %s", tag, ret);
            /******************************************************************/
            } else if (!strcmp(command, "EXPUNGE")) {
                if (split_line.size == 2) {// 0 arguments
                    ret = IMAP_expunge(tag);
                } else {
                    ret = format("%s %s", tag, "BAD Invalid arguments\n");
                }
            /******************************************************************/
            } else if (!strcmp(command, "FETCH")) {
                if (split_line.size == 4) {// 2 arguments
                    ret = IMAP_fetch(tag, split_line.data[2],
                          split_line.data[3]);
                } else {
                    ret = format("%s %s", tag, "BAD Invalid arguments\n");
                }
            /******************************************************************/
            } else if (!strcmp(command, "STORE")) {
                if (split_line.size == 5) {// 3 arguments
                    ret = IMAP_store(tag, split_line.data[2],
                          split_line.data[3], split_line.data[4]);
                } else {
                    ret = format("%s %s", tag, "BAD Invalid arguments\n");
                }
            /******************************************************************/
            } else if (!strcmp(command, "UID")) {
                if (split_line.size > 2) {
                    char *subcommand = split_line.data[2];
                    subcommand = upper_case(subcommand);
                    /**********************************************************/
                    if (!strcmp(subcommand, "FETCH")) {
                        if (split_line.size == 5) {// 2 arguments
                            ret = IMAP_uid_fetch(tag, split_line.data[3],
                                  split_line.data[4]);
                        } else {
                            ret = format("%s %s", tag, "BAD Invalid arguments\n");
                        }
                    /**********************************************************/
                    } else if (!strcmp(subcommand, "STORE")) {
                        if (split_line.size == 6) {// 3 arguments
                            ret = IMAP_uid_store(tag, split_line.data[3],
                                  split_line.data[4], split_line.data[5]);
                        } else {
                            ret = format("%s %s", tag, "BAD Invalid arguments\n");
                        }
                    /**********************************************************/
                    } else if (!strcmp(subcommand, "MOVE")) {
                        if (split_line.size == 5) {// 2 arguments
                            ret = IMAP_uid_move(split_line.data[3],
                                  split_line.data[4]);
                        } else {
                            ret = "BAD Invalid arguments\n";
                        }
                        ret = format("%s %s", tag, ret);
                    } else {
                        ret = format("%s BAD No such command as UID \"%s\"\n", tag, subcommand);
                    }
                } else {
                    ret = format("%s %s", tag, "BAD Invalid arguments\n");
                }
            /******************************************************************/
            } else {
                ret = format("%s BAD No such command as \"%s\"\n", tag, command);
            }
        }
    }
    free_vector(split_line);
    pop_stack();
    return ret;
}

/******************************************************************************/
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
            char line[MAX_LENGTH];
            ssize_t line_size;
            char *result = IMAP_capability("*");
            write(connection_socket, format("%s\n", result), strlen(result));
            while ((line_size = read(connection_socket, line, MAX_LENGTH-1)) > 0) {
                // Client sends a command
                line[line_size] = 0;
                result = IMAP_execute(line);
                write(connection_socket, format("%s\n", result), strlen(result));
                if (!is_online) exit(0);
            }
            exit(0);
        } else {// Parent Process
            close(connection_socket);
        }
    }
    return 0;
}

/******************************************************************************/

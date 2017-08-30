################################################################################
#                                IME-USP (2017)                                #
#        MAC0352 - Redes de Computadores e Sistemas Distribuidos - EP1         #
#                                                                              #
#                                   Makefile                                   #
#                                                                              #
#                       Pedro Pereira     - NUSP 9778794                       #
#                       Raphael R. Gusmao - NUSP 9778561                       #
################################################################################

.PHONY: clean
CC = gcc
CFLAGS = -Wall -std=c99 -O2 -g
OBJS = \
	error_handler.o \
	_aux.o \
	tcp_handler.o \
	imap_commands.o \
	imap_server.o

all: ep1

ep1: $(OBJS)
	$(CC) $(CFLAGS) $^ -o $@
	make clean

%.o: %.c %.h
	$(CC) $(CFLAGS) -c $< -o $@

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f *.o *~

################################################################################

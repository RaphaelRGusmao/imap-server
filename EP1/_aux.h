/******************************************************************************
 *                               IME-USP (2017)                               *
 *       MAC0352 - Redes de Computadores e Sistemas Distribuidos - EP1        *
 *                                                                            *
 *                             Auxiliar functions                             *
 *                                                                            *
 *                      Pedro Pereira     - NUSP 9778794                      *
 *                      Raphael R. Gusmao - NUSP 9778561                      *
 ******************************************************************************/

#ifndef __AUX_H__
#define __AUX_H__

#define bool char
#define true 1
#define false 0
#define MAX_LENGTH_CONSTANT 2048

typedef struct {
    int size;
    char **data;
} string_vector;

// Splits the string into a vector of instances before and after the divisor
string_vector split (const char *to_split, char* divisor);

// Prints a string_vector (for debug)
void print_string_vector (string_vector sv);

// Frees a string_vector
void free_vector(string_vector to_free);

// Converts a string to upper case
char *upper_case (char *s);

#endif

/******************************************************************************/

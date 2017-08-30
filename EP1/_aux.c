/******************************************************************************
 *                               IME-USP (2017)                               *
 *       MAC0352 - Redes de Computadores e Sistemas Distribuidos - EP1        *
 *                                                                            *
 *                             Auxiliar functions                             *
 *                                                                            *
 *                      Pedro Pereira     - NUSP 9778794                      *
 *                      Raphael R. Gusmao - NUSP 9778561                      *
 ******************************************************************************/

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include "error_handler.h"
#include "_aux.h"

string_vector split (const char *to_split, char *divisors)
{
    add_to_stack("aux -> split()");

    int len_to_split = strlen(to_split);
    int len_divisors = strlen(divisors);
    int flag = 0;
    int count = 0;
    for (int i = 0; i < len_to_split; i++) {
        char cur = to_split[i];
        for (int j = 0; j < len_divisors; j++) {
            if (cur == divisors[j]) {
                flag = -1;
                break;
            }
        }
        flag++;
        if (flag == 1) count++;
    }

    string_vector v;
    v.size = count;
    v.data = emalloc(count * sizeof(char *));

    count = 0;
    char *runner = strdup(to_split);
    char *start = runner;
    while (runner != NULL) {
        char *splitted_string = strdup(strsep(&runner, divisors));
        if (strcmp(splitted_string, "") != 0) {
            v.data[count++] = splitted_string;
        }
    }
    free(start);

    pop_stack();
    return v;
}

void print_string_vector (string_vector sv)
{
    printf("{\n  size:%d\n", sv.size);
    for (int i = 0; i < sv.size; i++) {
        printf("  %d:{%s}\n", i, sv.data[i]);
    }
    printf("}\n");
}

void free_vector (string_vector to_free)
{
    for (int i = 0; i < to_free.size; i++) {
        free(to_free.data[i]);
    }
    free(to_free.data);
}

char *upper_case (char *s)
{
    add_to_stack("aux -> upper_case()");
    char *up = emalloc(strlen(s) * sizeof(char));
    for (int i = 0; i < strlen(s); i++) {
        up[i] = toupper(s[i]);
    }
    return up;
    pop_stack();
}

/******************************************************************************/

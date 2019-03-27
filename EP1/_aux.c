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
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include "error_handler.h"
#include "_aux.h"

#define MAX_LENGTH 4096

/******************************************************************************/
string_vector split (const char *to_split, char *divisors)
{
    add_to_stack("aux -> split()");
    string_vector v;
    v.data = emalloc(64 * sizeof(char *));
    int len_to_split = strlen(to_split);
    int len_divisors = strlen(divisors);
    int ignore = 0;
    int flag = 0;
    int count = 0;
    int begin = 0, len = 0;
    for (int i = 0; i < len_to_split; i++) {
        char cur = to_split[i];
        if (cur == '(' || cur == '[' || cur == '{') {
            ignore++;
        } else if (cur == ')' || cur == ']' || cur == '}') {
            ignore--;
        }
        if (ignore == 0) {
            for (int j = 0; j < len_divisors; j++) {
                if (cur == divisors[j]) {
                    flag = -1;
                    break;
                }
            }
        }
        flag++;
        if (flag == 1) {
            count++;
            begin = i;
        } else if (len > 0 && flag == 0) {
            v.data[count-1] = substr(to_split, begin, len);
        }
        len = flag;
    }
    if (len > 0) v.data[count-1] = substr(to_split, begin, len);
    v.size = count;
    pop_stack();
    return v;
}

/******************************************************************************/
void print_string_vector (string_vector sv)
{
    printf("{\n  size:%d\n", sv.size);
    for (int i = 0; i < sv.size; i++) {
        printf("  %d:{%s}\n", i, sv.data[i]);
    }
    printf("}\n");
}

/******************************************************************************/
void free_vector (string_vector to_free)
{
    for (int i = 0; i < to_free.size; i++) {
        free(to_free.data[i]);
    }
    free(to_free.data);
}

/******************************************************************************/
char *substr (char *s, int begin, int len)
{
    return strndup(s + begin, len);
}

/******************************************************************************/
char *upper_case (char *s)
{
    add_to_stack("aux -> upper_case()");
    char *up = emalloc((strlen(s)+1) * sizeof(char));
    for (int i = 0; i < strlen(s); i++) {
        up[i] = toupper(s[i]);
    }
    up[strlen(s)] = 0;
    pop_stack();
    return up;
}

/******************************************************************************/
char *format (const char* expression, ...)
{
    char *ret = emalloc(8*MAX_LENGTH * sizeof(char));
    va_list args;
    va_start(args, expression);
    vsnprintf(ret, 8*MAX_LENGTH, expression, args);
    va_end(args);
    return ret;
}

/******************************************************************************/

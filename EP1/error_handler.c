/******************************************************************************
 *                               IME-USP (2017)                               *
 *       MAC0352 - Redes de Computadores e Sistemas Distribuidos - EP1        *
 *                                                                            *
 *                           Error handler archive                            *
 *                                                                            *
 *                      Pedro Pereira     - NUSP 9778794                      *
 *                      Raphael R. Gusmao - NUSP 9778561                      *
 ******************************************************************************/

#include <stdarg.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "error_handler.h"

struct function_stack {
    char func_name[32];
    struct function_stack* next;
};
typedef struct function_stack* stack;
static stack head = NULL;
static char name[1024] = "";
static int program_priority = 0;

/******************************************************************************/
void print_function_stack ()
{
    stack next;
    for (stack i = head; i != NULL; i = next) {
        next = i->next;
        printf("on %s:\n", i->func_name);
        free(i);
    }
}

/******************************************************************************/
stack create_stack ()
{
    stack s = emalloc(sizeof(struct function_stack));
    s->next = NULL;
    return s;
}

/******************************************************************************/
void add_to_stack (const char* function_name)
{
    if (head == NULL) {
        head = create_stack();
    } else {
        stack s = emalloc(sizeof(struct function_stack));
        s->next = head;
        head = s;
    }
    strncpy(head->func_name, function_name, 32);
    head->func_name[31] = '\0';
}

/******************************************************************************/
void pop_stack ()
{
    if (head == NULL) {
        add_to_stack("error_handler -> pop_stack()");
        die_with_msg("pop_stack: Stack is null");
    }
    stack i = head;
    head = head->next;
    free(i);
}

/******************************************************************************/
void die_with_msg (const char* expression, ...)
{
    char error_msg[1024];
    va_list args;
    va_start(args, expression);
    vsnprintf(error_msg, 1024, expression, args);
    va_end(args);
    printf("In program %s:\n", name);
    printf("During the program, a fatal error ocurred:\n");
    print_function_stack();
    fprintf(stderr, "\t%s: %s\n", name, error_msg);
    exit(-1);
}

/******************************************************************************/
void debug_print (int priority, const char* expression, ...)
{
    if (priority < program_priority) return;
    char error_msg[1024];
    va_list args;
    va_start(args, expression);
    vsnprintf(error_msg, 1024, expression, args);
    va_end(args);
    printf("debug.%d \t%s\n", priority, error_msg);
}

/******************************************************************************/
void* emalloc (size_t size)
{
    void *alloc = malloc(size);
    if (alloc == NULL) {
        add_to_stack("error_handler -> emalloc()");
        die_with_msg("Call to emalloc failed: Out of memory");
    }
    return alloc;
}

/******************************************************************************/
void set_program_name (const char* program_name)
{
    strncpy(name, program_name, 1023);
    name[1023] = '\0';
}

/******************************************************************************/
void set_debug_priority (int priority)
{
    program_priority = priority;
}

/******************************************************************************/

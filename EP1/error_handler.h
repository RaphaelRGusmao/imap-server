/******************************************************************************
 *                               IME-USP (2017)                               *
 *       MAC0352 - Redes de Computadores e Sistemas Distribuidos - EP1        *
 *                                                                            *
 *                           Error handler archive                            *
 *                                                                            *
 *                      Pedro Pereira     - NUSP 9778794                      *
 *                      Raphael R. Gusmao - NUSP 9778561                      *
 *                     Based in codes from Fernando Mario                     *
 ******************************************************************************/

#ifndef __ERROR_HANDLER__
#define __ERROR_HANDLER__
#include <stddef.h>

void print_function_stack ();

// Add function to stack of functions to print on error
void add_to_stack (const char* function_name);

// Pops stack. Will not print popped function on error
void pop_stack ();

// Like printf, prints error message and kills program
void die_with_msg (const char* expression, ...);

// Prints debug only if priority is higher than current priority
void debug_print (int priority, const char* expression, ...);

// Allocates memory, kills program if insuficient memory
void* emalloc (size_t size);

// On error message, will print program name
void set_program_name (const char* program_name);

// Sets the debug priority for debug_print
void set_debug_priority (int priority);

#endif

/******************************************************************************/

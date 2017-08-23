#ifndef __ERROR_HANDLER__
#define __ERROR_HANDLER__
#include <stddef.h>

/* Allocates memory, kills program if insuficient memory. */
void* emalloc(size_t size);

/* Like printf, prints error message and kills program. */
void die_with_msg(const char* expression, ...);

/* On error message, will print program name */
void set_program_name(const char* program_name);

/* Add function to stack of functions to print on error. */
void add_to_stack(const char* function_name);

/* Sets the debug priority for debug_print */
void set_debug_priority(int priority);

/* Prints debug only if priority is higher than current priority */
void debug_print(int priority, const char* expression, ...);

/* Pops stack. Will not print popped function on error. */
void pop_stack();

#endif

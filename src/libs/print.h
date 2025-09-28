#ifndef PRINT_H
#define PRINT_H

#include "file.h"
#include "litteral.h"

// File-based print functions using file.h
void print_string(file_t file, string string);
void print_format(file_t file, string format, ...);

// Stack allocator-based print functions
void* print_format_to_buffer(stack_alloc* alloc, string format, ...);

#endif /* PRINT_H */

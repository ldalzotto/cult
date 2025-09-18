#ifndef PRINT_H
#define PRINT_H

#include "file.h"
#include "litteral.h"

// File-based print functions using file.h
void print_string(file_t file, string_span string);
void print_format(file_t file, string_span format, ...);

// Stack allocator-based print functions
void* print_format_to_buffer(stack_alloc* alloc, string_span format, ...);

#endif /* PRINT_H */

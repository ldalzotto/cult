#include "print.h"
#include "convert.h"
#include "./assert.h"
#include "./format_iterator.h"
#include <stdarg.h>  // for variadic functions
#include <string.h>  // for memcpy

// Print a plain string to file
void print_string(file_t file, const char* str) {
    uptr len = 0;
    while (str[len] != '\0') ++len;
    file_write(file, str, len);
}

// Structure to hold output context for buffer output
typedef struct {
    stack_alloc* alloc;
} buffer_output_context;





// Process a format specifier and get the result
static void process_format_specifier(char specifier, va_list args, char* buffer, uptr* length) {
    switch (specifier) {
        case 'd': {
            // Handle signed integer
            i32 value = va_arg(args, i32);
            convert_i32_to_string(value, buffer, length);
            break;
        }
        case 'u': {
            // Handle unsigned integer
            u32 value = va_arg(args, u32);
            convert_u32_to_string(value, buffer, length);
            break;
        }
        case 's': {
            // Handle string
            const char* str = va_arg(args, const char*);
            if (str) {
                *length = 0;
                while (str[*length] != '\0') ++(*length);
                // Copy string to buffer
                for (uptr i = 0; i < *length; i++) {
                    buffer[i] = str[i];
                }
            } else {
                // Copy "(null)" to buffer
                const char* null_str = "(null)";
                *length = 6;
                for (uptr i = 0; i < *length; i++) {
                    buffer[i] = null_str[i];
                }
            }
            break;
        }
        case 'c': {
            // Handle character
            char c = (char)va_arg(args, int);  // char is promoted to int
            buffer[0] = c;
            *length = 1;
            break;
        }
        case 'p': {
            // Handle pointer
            void* ptr = va_arg(args, void*);
            convert_pointer_to_string(ptr, buffer, length);
            break;
        }
        default:
            // Handle unknown format specifier
            buffer[0] = '%';
            buffer[1] = specifier;
            *length = 2;
            break;
    }
}

// Print a formatted string with arguments to file
void print_format(file_t file, const char* format, ...) {
    debug_assert(format != 0);

    u8 stack[2048];
    stack_alloc alloc;
    sa_init(&alloc, stack, byteoffset(stack, sizeof(stack)));

    va_list args;
    va_start(args, format);

    format_iterator* iter = format_iterator_init(&alloc, format, args);

    while (1) {
        format_iteration fi = format_iterator_next(iter);
        if (fi.type == FORMAT_ITERATION_END) break;

        if (fi.type == FORMAT_ITERATION_LITERAL) {
            file_write(file, fi.text, fi.length);
        } else if (fi.type == FORMAT_ITERATION_SPECIFIER) {
            char buffer[256];
            uptr length;
            process_format_specifier(fi.specifier, args, buffer, &length);
            file_write(file, buffer, length);
        }
    }

    format_iterator_deinit(&alloc, iter);
    va_end(args);
}

// Print a formatted string with arguments to a buffer using stack allocator
void* print_format_to_buffer(stack_alloc* alloc, const char* format, ...) {
    debug_assert(alloc != 0);
    debug_assert(format != 0);

    void* start = alloc->cursor;

    va_list args;
    va_start(args, format);

    format_iterator* iter = format_iterator_init(alloc, format, args);

    while (1) {
        format_iteration fi = format_iterator_next(iter);
        if (fi.type == FORMAT_ITERATION_END) break;

        if (fi.type == FORMAT_ITERATION_LITERAL) {
            void* dest = sa_alloc(alloc, fi.length);
            memcpy(dest, fi.text, fi.length);
        } else if (fi.type == FORMAT_ITERATION_SPECIFIER) {
            char buffer[256];
            uptr length;
            process_format_specifier(fi.specifier, args, buffer, &length);
            void* dest = sa_alloc(alloc, length);
            memcpy(dest, buffer, length);
        }
    }

    format_iterator_deinit(alloc, iter);
    va_end(args);

    return start;
}

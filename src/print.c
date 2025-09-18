#include "print.h"
#include "./assert.h"
#include "./format_iterator.h"
#include <stdarg.h>  // for variadic functions

// Print a plain string to file
void print_string(file_t file, const string_span string) {
    file_write(file, string.begin, string.end);
}

// Print a formatted string with arguments to file
void print_format(file_t file, string_span format, ...) {
    u8 stack[2048];
    stack_alloc alloc;
    sa_init(&alloc, stack, byteoffset(stack, sizeof(stack)));

    va_list args;
    va_start(args, format);

    format_iterator* iter = format_iterator_init(&alloc, format, args);

    while (1) {
        format_iteration fi = format_iterator_next(iter);
        if (fi.type == FORMAT_ITERATION_END) break;

        file_write(file, fi.text.begin, fi.text.end);
    }

    format_iterator_deinit(&alloc, iter);
    va_end(args);
}

// Print a formatted string with arguments to a buffer using stack allocator
void* print_format_to_buffer(stack_alloc* alloc, string_span format, ...) {
    debug_assert(alloc != 0);

    void* start = alloc->cursor;

    va_list args;
    va_start(args, format);

    format_iterator* iter = format_iterator_init(alloc, format, args);

    while (1) {
        format_iteration fi = format_iterator_next(iter);
        if (fi.type == FORMAT_ITERATION_END) break;

        uptr text_length = bytesize(fi.text.begin, fi.text.end);
        void* dest = sa_alloc(alloc, text_length);
        sa_copy(alloc, fi.text.begin, dest, text_length);
    }

    format_iterator_deinit(alloc, iter);
    va_end(args);

    return start;
}

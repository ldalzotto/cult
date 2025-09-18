#ifndef FORMAT_ITERATOR_H
#define FORMAT_ITERATOR_H

#include "./stack_alloc.h"
#include "./litteral.h"
#include <stdarg.h>

typedef struct format_iterator format_iterator;
format_iterator* format_iterator_init(stack_alloc* alloc, string_span format, va_list args);
void format_iterator_deinit(stack_alloc* alloc, format_iterator* iterator);

typedef enum {
    FORMAT_ITERATION_LITERAL,
    FORMAT_ITERATION_CONTINUE,
    FORMAT_ITERATION_END
} format_iteration_type;

typedef struct {
    format_iteration_type type;
    struct {
        const char* begin;
        const char* end;
    } text;
} format_iteration;

format_iteration format_iterator_next(format_iterator* iterator);

#endif /* FORMAT_ITERATOR_H */

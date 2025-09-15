#ifndef PRINT_META_ITERATOR_H
#define PRINT_META_ITERATOR_H

#include "./stack_alloc.h"
#include "./print.h"

typedef struct print_meta_iterator print_meta_iterator;
print_meta_iterator* print_meta_iterator_init(stack_alloc* alloc, const print_meta* start);
void print_meta_iterator_deinit(stack_alloc* alloc, print_meta_iterator* iterator);

typedef struct {
    const print_meta* meta;
    const field_descriptor* fields_current;
} print_meta_iteration;
print_meta_iteration print_meta_iterator_next(print_meta_iterator* iterator);


#endif /* PRINT_META_ITERATOR_H */

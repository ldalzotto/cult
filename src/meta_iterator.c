#include "meta_iterator.h"
#include "assert.h"

typedef struct {
    const meta* meta;
    const field_descriptor* field_cursor;
} stack_entry;

typedef print_meta_iterator iterator;
struct print_meta_iterator {
    stack_entry* stack_begin;
    stack_entry* stack_end;
    void* stop;
    stack_entry* current;
};

static uptr iterator_max_depth(stack_alloc* alloc, const meta* meta) {
    void* begin = alloc->cursor;
    uptr depth_max = 0;
    typedef struct {
        const struct meta* meta;
        uptr depth;
    } entry;
    entry* start = sa_alloc(alloc, sizeof(*start));
    start->meta = meta;
    start->depth = 1;
    entry* stack_cursor = start;
    while ((void*)stack_cursor >= begin) {
        // print_meta* current = *(print_meta**)alloc->cursor;
        uptr depth_current = stack_cursor->depth;
        if (stack_cursor->depth > depth_max) {
            depth_max = stack_cursor->depth;
            
        }
        const struct meta* meta = stack_cursor->meta;

        sa_free(alloc, stack_cursor);
        stack_cursor -= 1;

        for (const field_descriptor* field = meta->fields.begin; field < meta->fields.end; ++field) {
            stack_cursor = sa_alloc(alloc, sizeof(*stack_cursor));
            stack_cursor->meta = field->field_meta;
            stack_cursor->depth = depth_current + 1;
        }
    }

    sa_free(alloc, start);

    return depth_max;
}

static iterator* iterator_init(stack_alloc* alloc, const meta* start) {
    iterator* it = sa_alloc(alloc, sizeof(iterator));

    uptr depth_max = iterator_max_depth(alloc, start);

    debug_assert(byteoffset(it, sizeof(*it)) == alloc->cursor);
    it->stop = alloc->cursor;
    it->stack_begin = sa_alloc(alloc, sizeof(stack_entry) * depth_max);
    it->stack_end = alloc->cursor;

    it->current = it->stack_begin;
    it->current->meta = start;
    it->current->field_cursor = start->fields.begin;
    return it;
}

static void iterator_deinit(stack_alloc* alloc, iterator* iterator) {
    sa_free(alloc, iterator);
}

static print_meta_iteration iterator_next(iterator* iterator) {
    print_meta_iteration iteration = {
        .meta = 0,
        .fields_current = 0,
    };
    stack_entry* entry = iterator->current;
    if ((void*)entry < iterator->stop) {
        return iteration;
    }
    stack_entry* result = iterator->current;

    iteration.fields_current = result->field_cursor;

    if (result->field_cursor != result->meta->fields.end) {
        const meta* meta_push = result->field_cursor->field_meta;
        result->field_cursor = byteoffset(result->field_cursor, sizeof(*result->field_cursor));
        iterator->current = byteoffset(iterator->current, sizeof(*iterator->current));
        stack_entry* entry_next = iterator->current;
        entry_next->meta = meta_push;
        entry_next->field_cursor = meta_push->fields.begin;
    } else {
        iterator->current -= 1;
    }

    iteration.meta = result->meta;
    return iteration;
}

/////////////////////////

print_meta_iterator* print_meta_iterator_init(stack_alloc* alloc, const meta* start) {
    return iterator_init(alloc, start);
}

void print_meta_iterator_deinit(stack_alloc* alloc, print_meta_iterator* iterator) {
    iterator_deinit(alloc, iterator);
}

print_meta_iteration print_meta_iterator_next(print_meta_iterator* iterator) {
    return iterator_next(iterator);
}

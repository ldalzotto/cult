#include <stddef.h>
#include "./stack_alloc.h"
#include "./assert.h"

// Initialize the stack allocator
void sa_init(stack_alloc* alloc, void* begin, void* end) {
    alloc->begin = begin;
    alloc->end = end;
    alloc->cursor = begin;
}

// Deinitialize the stack allocator by checking that the cursor is back to begin
void sa_deinit(stack_alloc* alloc) {
    debug_assert(alloc->cursor == alloc->begin);
}

// Allocate a block of memory
void* sa_alloc(stack_alloc* alloc, uptr size) {
    // Calculate new current position
    void* new_current = byteoffset(alloc->cursor, size);

    // Check if we have enough space
    debug_assert(new_current > alloc->cursor && new_current <= alloc->end);

    // Allocate by moving current
    void* result = alloc->cursor;
    alloc->cursor = new_current;
    return result;
}

// Free memory by rolling back the current allocation pointer to the specified pointer
void sa_free(stack_alloc* alloc, void* pointer) {
    debug_assert(pointer <= alloc->cursor);
    debug_assert(pointer >= alloc->begin);

    alloc->cursor = pointer;
}

#include <string.h>
#include "stack_alloc.h"
#include "assert.h"

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
    debug_assert(new_current >= alloc->cursor && new_current <= alloc->end);

    // Allocate by moving current
    void* result = alloc->cursor;
    alloc->cursor = new_current;
    return result;
}

// Free memory by rolling back the current allocation pointer to the specified pointer
void sa_free(stack_alloc* alloc, void* pointer) {
    debug_assert((uptr)pointer <= (uptr)alloc->cursor);
    debug_assert((uptr)pointer >= (uptr)alloc->begin);

    alloc->cursor = pointer;
}

// Move a block of memory from 'from' to 'to' within the stack allocator
void sa_move_tail(stack_alloc* alloc, void* from, void* to) {
    debug_assert(from >= alloc->begin && from <= alloc->cursor);
    uptr len = bytesize(from, alloc->cursor);
    debug_assert(to >= alloc->begin && byteoffset(to, len) <= alloc->end);
    memmove(to, from, len);
    alloc->cursor = byteoffset(to, len);
}

// Move a block of memory from 'from' to 'to' with specified size within the stack allocator
void sa_move(stack_alloc* alloc, void* from, void* to, uptr size) {
    debug_assert((uptr)from >= (uptr)alloc->begin && (uptr)from <= (uptr)alloc->cursor);
    debug_assert((uptr)byteoffset(from, size) <= (uptr)alloc->cursor);  // Ensure the block to move is within allocated memory
    debug_assert((uptr)to >= (uptr)alloc->begin && (uptr)byteoffset(to, size) <= (uptr)alloc->end);

    memmove(to, from, size);

    // Note: cursor is never updated in sa_move (unlike sa_move_tail)
}

// Copy a block of memory from 'from' to 'to' with specified size, where 'to' is within the stack allocator
void sa_copy(stack_alloc* alloc, const void* from, void* to, uptr size) {
    debug_assert(!((uptr)from >= (uptr)alloc->begin && (uptr)from <= (uptr)alloc->cursor) || ((uptr)byteoffset(from, size) <= (uptr)alloc->cursor));
    debug_assert((uptr)to >= (uptr)alloc->begin && (uptr)byteoffset(to, size) <= (uptr)alloc->end);

    __builtin_memcpy(to, from, size);

    // Note: cursor is never updated in sa_copy (similar to sa_move)
}

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
    unused(alloc);
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

// Insert a new block of memory of 'size' bytes at the address 'at' by shifting the tail.
void sa_insert(stack_alloc* alloc, void* at, uptr size) {
    // Record current end of allocated region
    void* old_cursor = alloc->cursor;
    // Reserve space for the new block
    sa_alloc(alloc, size);
    // Move the existing tail starting at 'at' forward by 'size' bytes
    uptr move_len = bytesize(at, old_cursor);
    sa_move(alloc, at, byteoffset(at, size), move_len);
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
    __builtin_memmove(to, from, len);
    alloc->cursor = byteoffset(to, len);
}

// Move a block of memory from 'from' to 'to' with specified size within the stack allocator
void sa_move(stack_alloc* alloc, void* from, void* to, uptr size) {
    debug_assert((uptr)from >= (uptr)alloc->begin && (uptr)from <= (uptr)alloc->cursor);
    debug_assert((uptr)byteoffset(from, size) <= (uptr)alloc->cursor);  // Ensure the block to move is within allocated memory
    debug_assert((uptr)to >= (uptr)alloc->begin && (uptr)byteoffset(to, size) <= (uptr)alloc->end);
    unused(alloc);

    __builtin_memmove(to, from, size);

    // Note: cursor is never updated in sa_move (unlike sa_move_tail)
}

// Copy a block of memory from 'from' to 'to' with specified size, where 'to' is within the stack allocator
void sa_copy(stack_alloc* alloc, const void* from, void* to, uptr size) {
    debug_assert(!((uptr)from >= (uptr)alloc->begin && (uptr)from <= (uptr)alloc->cursor) || ((uptr)byteoffset(from, size) <= (uptr)alloc->cursor));
    debug_assert((uptr)to >= (uptr)alloc->begin && (uptr)byteoffset(to, size) <= (uptr)alloc->end);
    unused(alloc);

    __builtin_memcpy(to, from, size);

    // Note: cursor is never updated in sa_copy (similar to sa_move)
}

// Compare two memory ranges for equality
u8 sa_equals(stack_alloc* alloc, const void* left_begin, const void* left_end, const void* right_begin, const void* right_end) {

    // If left range is within the allocator, ensure it's within allocated memory
    debug_assert(!((uptr)left_begin >= (uptr)alloc->begin && (uptr)left_begin <= (uptr)alloc->cursor) || ((uptr)left_end <= (uptr)alloc->cursor));

    // If right range is within the allocator, ensure it's within allocated memory
    debug_assert(!((uptr)right_begin >= (uptr)alloc->begin && (uptr)right_begin <= (uptr)alloc->cursor) || ((uptr)right_end <= (uptr)alloc->cursor));

    unused(alloc);

    uptr left_size = bytesize(left_begin, left_end);
    uptr right_size = bytesize(right_begin, right_end);
    if (left_size != right_size) {return 0;}
    return __builtin_memcmp(left_begin, right_begin, right_size) == 0 ? 1 : 0;
}

// Check if a needle memory range is contained within a haystack memory range
u8 sa_contains(stack_alloc* alloc, const void* haystack_begin, const void* haystack_end, const void* needle_begin, const void* needle_end) {
    if (sa_find(alloc, haystack_begin, haystack_end, needle_begin, needle_end)) {
        return 1;
    }
    return 0;
}

void* sa_find(stack_alloc* alloc, const void* haystack_begin, const void* haystack_end, const void* needle_begin, const void* needle_end) {
    uptr haystack_size = (uptr)((char*)haystack_end - (char*)haystack_begin);
    uptr needle_size = (uptr)((char*)needle_end - (char*)needle_begin);

    if (needle_size > haystack_size) return 0;

    // If haystack is within the allocator, ensure it's within allocated memory
    debug_assert(!((uptr)haystack_begin >= (uptr)alloc->begin && (uptr)haystack_begin <= (uptr)alloc->cursor) || ((uptr)haystack_end <= (uptr)alloc->cursor));

    // If needle is within the allocator, ensure it's within allocated memory
    debug_assert(!((uptr)needle_begin >= (uptr)alloc->begin && (uptr)needle_begin <= (uptr)alloc->cursor) || ((uptr)needle_end <= (uptr)alloc->cursor));

    unused(alloc);

    for (uptr i = 0; i <= haystack_size - needle_size; i++) {
        if (__builtin_memcmp((char*)haystack_begin + i, needle_begin, needle_size) == 0) {
            return byteoffset(haystack_begin, i);
        }
    }

    return 0;
}

// Set a memory range to a specified byte value
void sa_set(stack_alloc* alloc, void* begin, void* end, u8 value) {

    uptr size = (uptr)((char*)end - (char*)begin);

    // If range is within the allocator, ensure it's within allocated memory
    debug_assert(!((uptr)begin >= (uptr)alloc->begin && (uptr)begin <= (uptr)alloc->cursor) || ((uptr)end <= (uptr)alloc->cursor));

    unused(alloc);

    __builtin_memset(begin, value, size);
}

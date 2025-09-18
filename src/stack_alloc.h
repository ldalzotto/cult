#ifndef STACK_ALLOC_H
#define STACK_ALLOC_H

#include "primitive.h"

// Stack allocator module for linear memory allocation/deallocation
//
// Initialized with begin and end pointers to a pre-allocated memory block.
// Provides stack-like allocation: allocations are linear, deallocation is by reset.
//
// Example usage:
//   void* memory = mem_map(1024);
//   Stack_Allocator alloc;
//   stack_alloc_init(&alloc, memory, (void*)((char*)memory + 1024));
//   void* ptr = stack_alloc_allocate(&alloc, 64);
//   // ... use ptr ...
//   stack_alloc_reset(&alloc);  // optional reset to deallocate

typedef struct {
    void* begin;    // Start of the memory block
    void* end;      // End of the memory block
    void* cursor;  // Current allocation pointer
} stack_alloc;

// Initialize the stack allocator with begin and end pointers
//
// @param alloc: Pointer to Stack_Allocator to initialize
// @param begin: Start of the memory block
// @param end: End of the memory block
//
// Returns: void
// Preconditions: begin < end, and they represent a valid memory region
// Postconditions: alloc->current == begin
void sa_init(stack_alloc* alloc, void* begin, void* end);

// Deinitialize the stack allocator by checking that the cursor is back to begin
//
// @param alloc: Pointer to stack_alloc
//
// Returns: void
// Preconditions: alloc->cursor == alloc->begin (failure is assertion error)
void sa_deinit(stack_alloc* alloc);

// Allocate a block of memory of the given size
//
// @param alloc: Pointer to Stack_Allocator
// @param size: Size in bytes to allocate
//
// Returns: Pointer to allocated memory (asserts if out of memory)
void* sa_alloc(stack_alloc* alloc, uptr size);

void sa_free(stack_alloc* alloc, void* pointer);

// Move a block of memory from 'from' to 'to' within the stack allocator
//
// @param alloc: Pointer to stack_alloc
// @param from: Start of the memory block to move (must be within the allocated region)
// @param to: Destination where the block should be moved to
//
// Returns: void
// Preconditions: from must be between alloc->begin and alloc->cursor, to must be between alloc->begin
//                and alloc->end - (alloc->cursor - from), to ensure the block fits
// Postconditions: The memory block has been moved, alloc->cursor is updated to the new position
void sa_move_tail(stack_alloc* alloc, void* from, void* to);

// Move a block of memory from 'from' to 'to' with specified size within the stack allocator
//
// @param alloc: Pointer to stack_alloc
// @param from: Start of the memory block to move (must be within the allocated region)
// @param to: Destination where the block should be moved to
// @param size: Size in bytes of the memory block to move
//
// Returns: void
// Preconditions:
// - from must be between alloc->begin and alloc->cursor
// - from + size must not exceed alloc->cursor (the block must be allocated)
// - to must be between alloc->begin and alloc->end
// - to + size must not exceed alloc->end
// Postconditions: The memory block has been moved to the destination, alloc->cursor remains unchanged
void sa_move(stack_alloc* alloc, void* from, void* to, uptr size);

// Copy a block of memory from 'from' to 'to' with specified size, where 'to' is within the stack allocator
//
// @param alloc: Pointer to stack_alloc
// @param from: Start of the memory block to copy from (can be from anywhere; if within the allocator, must be within allocated region)
// @param to: Destination where the block should be copied to
// @param size: Size in bytes of the memory block to copy
//
// Returns: void
// Preconditions:
// - if from is between alloc->begin and alloc->cursor, then from + size must not exceed alloc->cursor
// - to must be between alloc->begin and alloc->end
// - to + size must not exceed alloc->end
// Postconditions: The memory block has been copied to the destination, alloc->cursor remains unchanged
void sa_copy(stack_alloc* alloc, const void* from, void* to, uptr size);

#endif /* STACK_ALLOC_H */

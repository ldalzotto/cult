#ifndef STACK_ALLOC_H
#define STACK_ALLOC_H

#include "./primitive.h"

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

// Free memory by rolling back the current allocation pointer to the specified pointer
//
// @param alloc: Pointer to stack_alloc
// @param pointer: The pointer to revert the current allocation to (must be within the allocated memory block)
//
// Returns: void
// Preconditions: pointer must be between alloc->begin and alloc->current
// Postconditions: alloc->current == pointer
void sa_free(stack_alloc* alloc, void* pointer);

#endif /* STACK_ALLOC_H */

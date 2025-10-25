#ifndef MEM_H
#define MEM_H

#include "primitive.h"

// Basic memory map/unmap functions
// mem_map: Allocates a block of memory of the specified size.
// Precondition: size must be greater than 0.
// Postcondition: Always returns a valid pointer (neither NULL nor MAP_FAILED).
void* mem_map(uptr size);

// mem_unmap: Deallocates the memory block pointed to by ptr
void mem_unmap(void* pointer, uptr size);

uptr mem_cstrlen(const void* pointer);

#endif /* MEM_H */

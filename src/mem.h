#ifndef MEM_H
#define MEM_H

#include "./primitive.h"

// Basic memory map/unmap functions
// mem_map: Allocates a block of memory of the specified size
void* mem_map(uptr size);

// mem_unmap: Deallocates the memory block pointed to by ptr
void mem_unmap(void* pointer, uptr size);

#endif /* MEM_H */

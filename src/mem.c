#include "mem.h"
#include "assert.h"

#include <sys/mman.h>

void* mem_map(uptr size) {
    void* pointer = mmap(0, size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    debug_assert(pointer != MAP_FAILED);
    return pointer;
}

void mem_unmap(void* pointer, uptr size) {
    i32 result = munmap(pointer, size);
    debug_assert(result == 0);
}

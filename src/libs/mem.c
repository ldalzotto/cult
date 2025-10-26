#include "mem.h"
#include "assert.h"

#include <sys/mman.h>
#include <unistd.h>

void* mem_map(uptr size) {
    void* pointer = mmap(0, size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    debug_assert(pointer != MAP_FAILED);
    return pointer;
}

void mem_unmap(void* pointer, uptr size) {
    i32 result = munmap(pointer, size);
    debug_assert(result == 0);
    unused(result);
}

static uptr mem_align_up_size(uptr n, uptr align) {
    uptr result = (n + align - 1) & ~(align - 1);
    return result;
}

void mem_release_unused(void* begin, void* end) {
    const uptr page = getpagesize();
    void* aligned = (void*)mem_align_up_size((uptr)begin, page);
    if (aligned < end) {
        const uptr len = bytesize(aligned, end);
        void *m = mmap(aligned, len, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS | MAP_FIXED, -1, 0);
        debug_assert(m != MAP_FAILED);
    }
}

uptr mem_cstrlen(const void* pointer) {
    const u8* cursor = pointer;
    while (*cursor) {++cursor;}
    return bytesize(pointer, cursor);
}

mem_bytesize_human_readable_values mem_bytesize_human_readable(void* begin, void* end) {
    const uptr size = bytesize(begin, end);
    const uptr mib = size / (1024 * 1024);
    const uptr kib = (size % (1024 * 1024)) / 1024;
    return (mem_bytesize_human_readable_values) {.mib = mib, .kib = kib};
}


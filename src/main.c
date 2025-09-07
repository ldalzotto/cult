#include <stdio.h>
#include "primitive.h"
#include "./mem.h"
#include "./stack_alloc.h"

static void memory_test() {
    // Simple test for stack_alloc
    uptr size = 1024;
    void* p = mem_map(size);
    
    stack_alloc alloc;
    sa_init(&alloc, p, byteoffset(p, size));

    // Allocate 64 bytes
    void* ptr1 = sa_alloc(&alloc, 64);
    printf("Allocated 64 bytes at address: %p\n", ptr1);

    // Try to allocate more than remaining space (200 - 64 = 136, allocate 160)
    void* ptr2 = sa_alloc(&alloc, 160);
    printf("Large allocation (160 bytes) success as expected: %p\n", ptr2);

    // Reset to beginning
    sa_free(&alloc, p);
    printf("Reset allocation pointer to beginning\n");

    // Allocate again, should reuse space
    void* ptr3 = sa_alloc(&alloc, 32);
    if (ptr3 == p) {
        printf("Allocation reused space correctly after reset\n");
    } else {
        printf("Allocation did not reuse space as expected\n");
    }

    sa_free(&alloc, p);

    sa_deinit(&alloc);

    mem_unmap(p, size);
}

i32 main() {
    

    // Test the primitive definitions
    i32 signed_32bit = -12345;
    u32 unsigned_32bit = 12345;

    printf("Hello, World!\n");
    printf("i32 value: %d\n", signed_32bit);
    printf("u32 value: %u\n", unsigned_32bit);

    memory_test();

    return 0;
}

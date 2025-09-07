#include <stdio.h>
#include "primitive.h"
#include "./mem.h"

i32 main() {
    uptr size = 200;
    void* p = mem_map(size);
    mem_unmap(p, size);
    // Test the primitive definitions
    i32 signed_32bit = -12345;
    u32 unsigned_32bit = 12345;

    printf("Hello, World!\n");
    printf("i32 value: %d\n", signed_32bit);
    printf("u32 value: %u\n", unsigned_32bit);

    return 0;
}

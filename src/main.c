#include <stdio.h>
#include "primitive.h"

int main() {
    // Test the primitive definitions
    i32 signed_32bit = -12345;
    u32 unsigned_32bit = 12345;

    printf("Hello, World!\n");
    printf("i32 value: %d\n", signed_32bit);
    printf("u32 value: %u\n", unsigned_32bit);

    return 0;
}

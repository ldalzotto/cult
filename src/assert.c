#include "./assert.h"
#include "./backtrace.h"
#include <stdio.h>

void __debug_assert(u8 condition, char* cond_str, char* file, int line) {
    if (!condition) {
        printf("ASSERT FAILED: %s at %s:%d\n", cond_str, file, line);
        print_backtrace();
        *(volatile u8*)0 = 1;
    }
}

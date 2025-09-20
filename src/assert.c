#include "assert.h"
#include "backtrace.h"
#include "print.h"

void __debug_assert(u8 condition, string cond_str, string file, int line) {
    if (!condition) {
        print_format(file_stderr(), STRING("ASSERT FAILED: %s at %s:%d\n"), cond_str, file, line);
        print_backtrace(file_stderr());
        *(volatile u8*)0 = 1;
    }
}

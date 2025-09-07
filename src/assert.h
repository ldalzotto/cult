#ifndef ASSERT_H
#define ASSERT_H

#include "./primitive.h"
#include <stdio.h>

#ifndef DEBUG_ASSERTIONS_ENABLED
#error DEBUG_ASSERTIONS_ENABLED not defined
#endif

#if DEBUG_ASSERTIONS_ENABLED
#define debug_assert __debug_assert
#else
#define debug_assert
#endif

static void __debug_assert(u8 condition) {
    if (!condition) {
        *(volatile u8*)0 = 1;
    }
}




#endif // ASSERT_H

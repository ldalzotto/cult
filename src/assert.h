#ifndef ASSERT_H
#define ASSERT_H

#include "./primitive.h"

#ifndef DEBUG_ASSERTIONS_ENABLED
#error DEBUG_ASSERTIONS_ENABLED not defined
#endif

#if DEBUG_ASSERTIONS_ENABLED
#define debug_assert(cond) __debug_assert(cond, #cond, __FILE__, __LINE__)
#else
#define debug_assert(cond)
#endif

void __debug_assert(u8 condition, char* cond_str, char* file, int line);

#endif // ASSERT_H

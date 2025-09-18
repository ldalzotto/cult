#ifndef ASSERT_H
#define ASSERT_H

#include "./primitive.h"
#include "./litteral.h"

#ifndef DEBUG_ASSERTIONS_ENABLED
#error DEBUG_ASSERTIONS_ENABLED not defined
#endif

#if DEBUG_ASSERTIONS_ENABLED
#define debug_assert(cond) __debug_assert(cond, STRING(#cond), STRING(__FILE__), __LINE__)
#else
#define debug_assert(cond)
#endif

void __debug_assert(u8 condition, string cond_str, string file, int line);

#endif // ASSERT_H

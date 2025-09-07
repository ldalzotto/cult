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

// Test framework macros and functions
extern int test_passed;
extern int test_failed;

static void test_reset() {
    test_passed = 0;
    test_failed = 0;
}

static void test_report() {
    printf("Test Results:\n");
    printf("  Passed: %d\n", test_passed);
    printf("  Failed: %d\n", test_failed);
    printf("  Total: %d\n", test_passed + test_failed);
}

// Test assertion macros (always active, independent of debug mode)
#define test_assert(cond, msg) \
    if (cond) { \
        test_passed++; \
    } else { \
        printf("TEST FAILED: %s\n", msg); \
        test_failed++; \
    }

// Convenient test macros
#define TEST_ASSERT_TRUE(expr) test_assert((expr), "Expected true: " #expr)
#define TEST_ASSERT_FALSE(expr) test_assert(!(expr), "Expected false: " #expr)
#define TEST_ASSERT_NULL(ptr) test_assert((ptr) == NULL, "Expected null: " #ptr)
#define TEST_ASSERT_NOT_NULL(ptr) test_assert((ptr) != NULL, "Expected not null: " #ptr)
#define TEST_ASSERT_EQUAL(a, b) test_assert((a) == (b), "Expected equal: " #a " == " #b)


#endif // ASSERT_H

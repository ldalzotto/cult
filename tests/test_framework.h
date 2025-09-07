#ifndef TEST_FRAMEWORK_H
#define TEST_FRAMEWORK_H

#include "../src/primitive.h"
#include <stdio.h>

typedef struct test_context {
    u32 passed;
    u32 failed;
} test_context;

static inline void test_reset_context(test_context* t) {
    t->passed = 0;
    t->failed = 0;
}

static inline void test_report_context(test_context* t) {
    printf("Test Results:\n");
    printf("  Passed: %u\n", t->passed);
    printf("  Failed: %u\n", t->failed);
    printf("  Total: %u\n", t->passed + t->failed);
}

// Test macros using context
#define TEST_ASSERT(ctx, cond, msg) \
    if ((cond)) { \
        (ctx)->passed++; \
    } else { \
        printf("TEST FAILED: %s at %s:%d\n", msg, __FILE__, __LINE__); \
        (ctx)->failed++; \
    }

#define TEST_ASSERT_TRUE(ctx, expr) TEST_ASSERT(ctx, (expr), "Expected true: " #expr)
#define TEST_ASSERT_FALSE(ctx, expr) TEST_ASSERT(ctx, !(expr), "Expected false: " #expr)
#define TEST_ASSERT_NULL(ctx, ptr) TEST_ASSERT(ctx, (ptr) == NULL, "Expected null: " #ptr)
#define TEST_ASSERT_NOT_NULL(ctx, ptr) TEST_ASSERT(ctx, (ptr) != NULL, "Expected not null: " #ptr)
#define TEST_ASSERT_EQUAL(ctx, a, b) TEST_ASSERT(ctx, (a) == (b), "Expected equal: " #a " == " #b)

#endif /* TEST_FRAMEWORK_H */

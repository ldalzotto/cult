#ifndef TEST_FRAMEWORK_H
#define TEST_FRAMEWORK_H

#include "../src/primitive.h"
#include "../src/backtrace.h"

typedef struct test_context {
    u32 passed;
    u32 failed;
} test_context;

// Function declarations
void test_reset_context(test_context* t);
void test_report_context(test_context* t);

// Test macros using context
#define TEST_ASSERT(ctx, cond, msg) \
    if ((cond)) { \
        (ctx)->passed++; \
    } else { \
        printf("TEST FAILED: %s at %s:%d\n", msg, __FILE__, __LINE__); \
        print_backtrace(); \
        (ctx)->failed++; \
    }

#define TEST_ASSERT_TRUE(ctx, expr) TEST_ASSERT(ctx, (expr), "Expected true: " #expr)
#define TEST_ASSERT_FALSE(ctx, expr) TEST_ASSERT(ctx, !(expr), "Expected false: " #expr)
#define TEST_ASSERT_NULL(ctx, ptr) TEST_ASSERT(ctx, (ptr) == NULL, "Expected null: " #ptr)
#define TEST_ASSERT_NOT_NULL(ctx, ptr) TEST_ASSERT(ctx, (ptr) != NULL, "Expected not null: " #ptr)
#define TEST_ASSERT_EQUAL(ctx, a, b) TEST_ASSERT(ctx, (a) == (b), "Expected equal: " #a " == " #b)

#endif /* TEST_FRAMEWORK_H */

#ifndef TEST_FRAMEWORK_H
#define TEST_FRAMEWORK_H

#include "../src/primitive.h"
#include "../src/backtrace.h"

// TODO: remove this and use the stack_alloc instead
#define MAX_TEST_NAME_LEN 64
#define MAX_TESTS 256

typedef struct test_context {
    u32 passed;
    u32 failed;
    char filter_pattern[MAX_TEST_NAME_LEN];
    u32 test_count;
    struct {
        char name[MAX_TEST_NAME_LEN];
        void (*func)(struct test_context* t);
    } tests[MAX_TESTS];
} test_context;

// Function declarations
void test_reset_context(test_context* t);
void test_report_context(test_context* t);
void test_register(test_context* t, const char* name, void (*func)(test_context* t));
void test_run_filtered(test_context* t);

// Test macros using context
#define TEST_ASSERT(ctx, cond, msg) \
    if ((cond)) { \
        (ctx)->passed++; \
    } else { \
        print_format(file_stdout(), "TEST FAILED: %s at %s:%d\n", msg, __FILE__, __LINE__); \
        print_backtrace(); \
        (ctx)->failed++; \
    }

#define TEST_ASSERT_TRUE(ctx, expr) TEST_ASSERT(ctx, (expr), "Expected true: " #expr)
#define TEST_ASSERT_FALSE(ctx, expr) TEST_ASSERT(ctx, !(expr), "Expected false: " #expr)
#define TEST_ASSERT_NULL(ctx, ptr) TEST_ASSERT(ctx, (ptr) == NULL, "Expected null: " #ptr)
#define TEST_ASSERT_NOT_NULL(ctx, ptr) TEST_ASSERT(ctx, (ptr) != NULL, "Expected not null: " #ptr)
#define TEST_ASSERT_EQUAL(ctx, a, b) TEST_ASSERT(ctx, (a) == (b), "Expected equal: " #a " == " #b)
#define TEST_ASSERT_NOT_EQUAL(ctx, a, b) TEST_ASSERT(ctx, (a) != (b), "Expected not equal: " #a " != " #b)

// Test registration macro
#define REGISTER_TEST(ctx, name, func) test_register(ctx, name, func)

#endif /* TEST_FRAMEWORK_H */

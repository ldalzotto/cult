#ifndef TEST_FRAMEWORK_H
#define TEST_FRAMEWORK_H

#include "primitive.h"
#include "litteral.h"
#include "stack_alloc.h"
#include "backtrace.h"

typedef struct test_context test_context;

typedef struct test_context_entry {
    void (*func)(struct test_context* t);
    string name;
} test_context_entry;

struct test_context {
    stack_alloc* alloc;
    u32 passed;
    u32 failed;
    u32 test_count;
    string filter_pattern;
    test_context_entry* entries;
};

// Function declarations
test_context* test_context_init(stack_alloc* alloc);
void test_report_context(test_context* t);
void test_register(test_context* t, const string name, void (*func)(test_context* t));
void test_run_filtered(test_context* t);

// Test macros using context
#define TEST_ASSERT(ctx, cond, msg) \
    if ((cond)) { \
        (ctx)->passed++; \
    } else { \
        print_format(file_stdout(), STRING("TEST FAILED: %s at %s:%d\n"), STRING(msg), STRING(__FILE__), __LINE__); \
        print_backtrace(); \
        (ctx)->failed++; \
    }

#define TEST_ASSERT_TRUE(ctx, expr) TEST_ASSERT(ctx, (expr), "Expected true: " #expr)
#define TEST_ASSERT_FALSE(ctx, expr) TEST_ASSERT(ctx, !(expr), "Expected false: " #expr)
#define TEST_ASSERT_NULL(ctx, ptr) TEST_ASSERT(ctx, (ptr) == 0, "Expected null: " #ptr)
#define TEST_ASSERT_NOT_NULL(ctx, ptr) TEST_ASSERT(ctx, (ptr) != 0, "Expected not null: " #ptr)
#define TEST_ASSERT_EQUAL(ctx, a, b) TEST_ASSERT(ctx, (a) == (b), "Expected equal: " #a " == " #b)
#define TEST_ASSERT_NOT_EQUAL(ctx, a, b) TEST_ASSERT(ctx, (a) != (b), "Expected not equal: " #a " != " #b)

// Test registration macro
#define REGISTER_TEST(ctx, name, func) test_register(ctx, STRING(name), func)

#endif /* TEST_FRAMEWORK_H */

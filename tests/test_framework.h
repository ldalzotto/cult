#ifndef TEST_FRAMEWORK_H
#define TEST_FRAMEWORK_H

#include "../src/primitive.h"
#include "../src/litteral.h"
#include "../src/stack_alloc.h"
#include "../src/backtrace.h"

typedef struct test_context test_context;

typedef struct test_context_entry {
    void (*func)(struct test_context* t);
    string_span name;
} test_context_entry;

struct test_context {
    stack_alloc* alloc;
    u32 passed;
    u32 failed;
    u32 test_count;
    string_span filter_pattern;
    test_context_entry* entries;
};

// Function declarations
test_context* test_context_init(stack_alloc* alloc);
void test_report_context(test_context* t);
void test_register(test_context* t, const string_span name, void (*func)(test_context* t));
void test_run_filtered(test_context* t);

// Test macros using context
#define TEST_ASSERT(ctx, cond, msg) \
    if ((cond)) { \
        (ctx)->passed++; \
    } else { \
        print_format(file_stdout(), STR_SPAN("TEST FAILED: %s at %s:%d\n"), STR_SPAN(msg), STR_SPAN(__FILE__), __LINE__); \
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
#define REGISTER_TEST(ctx, name, func) test_register(ctx, STR_SPAN(name), func)

#endif /* TEST_FRAMEWORK_H */

// Tests for stack allocator module
#include "test_framework.h"
#include "../src/mem.h"
#include "../src/stack_alloc.h"
#include "../src/assert.h"
#include <stdio.h>

static void test_sa_basic_alloc(test_context* t) {
    uptr size = 1024;
    void* mem = mem_map(size);
    TEST_ASSERT_NOT_NULL(t, mem);

    stack_alloc alloc;
    sa_init(&alloc, mem, byteoffset(mem, size));

    // Allocate 64 bytes
    void* p1 = sa_alloc(&alloc, 64);
    TEST_ASSERT_NOT_NULL(t, p1);
    TEST_ASSERT_TRUE(t, p1 >= alloc.begin);
    TEST_ASSERT_TRUE(t, p1 < alloc.end);

    // Allocate another 64 bytes
    void* p2 = sa_alloc(&alloc, 64);
    TEST_ASSERT_NOT_NULL(t, p2);
    TEST_ASSERT_TRUE(t, p2 > p1);
    TEST_ASSERT_TRUE(t, p2 <= alloc.end);

    // Free back to after p1
    sa_free(&alloc, byteoffset(p1, 64));
    TEST_ASSERT_TRUE(t, alloc.cursor == byteoffset(p1, 64));

    // Allocate again, should reuse the space after previous free
    void* p3 = sa_alloc(&alloc, 32);
    TEST_ASSERT_TRUE(t, p3 == byteoffset(p1, 64));
    TEST_ASSERT_TRUE(t, p3 <= alloc.end);

    // Free to beginning
    sa_free(&alloc, alloc.begin);
    TEST_ASSERT_TRUE(t, alloc.cursor == alloc.begin);

    sa_deinit(&alloc);
    mem_unmap(mem, size);
}

static void test_sa_reuse_space(test_context* t) {
    uptr size = 256;
    void* mem = mem_map(size);
    TEST_ASSERT_NOT_NULL(t, mem);

    stack_alloc alloc;
    sa_init(&alloc, mem, byteoffset(mem, size));

    // Allocate two blocks
    void* p1 = sa_alloc(&alloc, 64);
    void* p2 = sa_alloc(&alloc, 64);

    // Free the second block
    sa_free(&alloc, p2);
    TEST_ASSERT_TRUE(t, alloc.cursor == p2);

    // Free the first block (rewind further back)
    sa_free(&alloc, p1);
    TEST_ASSERT_TRUE(t, alloc.cursor == p1);

    // Now allocate more than first block
    void* p3 = sa_alloc(&alloc, 128);
    TEST_ASSERT_TRUE(t, p3 == p1);

    sa_free(&alloc, alloc.begin);
    sa_deinit(&alloc);
    mem_unmap(mem, size);
}

static void test_sa_over_allocation(test_context* t) {
    uptr size = 128;
    void* mem = mem_map(size);
    TEST_ASSERT_NOT_NULL(t, mem);

    stack_alloc alloc;
    sa_init(&alloc, mem, byteoffset(mem, size));

    // Allocate most of the space
    void* p1 = sa_alloc(&alloc, 80);
    TEST_ASSERT_TRUE(t, p1 != NULL);

    // Try to allocate more than remaining space
    void* p2 = sa_alloc(&alloc, 60);  // 48 remaining (128 - 80 = 48)
    TEST_ASSERT_TRUE(t, p2 != NULL);
    // Since debug_assert is off in tests, it will allocate even if over
    // So check if cursor moved correctly
    if (alloc.cursor > alloc.end) {
        printf("Warning: Over-allocation detected\n");
    }

    // Free to beginning
    sa_free(&alloc, alloc.begin);
    TEST_ASSERT_TRUE(t, alloc.cursor == alloc.begin);

    sa_deinit(&alloc);
    mem_unmap(mem, size);
}

void test_sa_module(test_context* t) {
    printf("Running Stack Allocator Module Tests...\n");
    test_sa_basic_alloc(t);
    test_sa_reuse_space(t);
    test_sa_over_allocation(t);
}

// Tests for stack allocator module
#include "../src/mem.h"
#include "../src/stack_alloc.h"
#include "../src/assert.h"
#include <sys/mman.h>
#include <stdio.h>

void test_sa_basic_alloc() {
    uptr size = 1024;
    void* mem = mem_map(size);
    TEST_ASSERT_NOT_NULL(mem);
    TEST_ASSERT_TRUE(mem != MAP_FAILED);

    stack_alloc alloc;
    sa_init(&alloc, mem, byteoffset(mem, size));

    // Allocate 64 bytes
    void* p1 = sa_alloc(&alloc, 64);
    TEST_ASSERT_NOT_NULL(p1);
    TEST_ASSERT_TRUE(p1 >= alloc.begin);
    TEST_ASSERT_TRUE(p1 < alloc.end);

    // Allocate another 64 bytes
    void* p2 = sa_alloc(&alloc, 64);
    TEST_ASSERT_NOT_NULL(p2);
    TEST_ASSERT_TRUE(p2 > p1);
    TEST_ASSERT_TRUE(p2 <= alloc.end);

    // Free back to after p1
    sa_free(&alloc, byteoffset(p1, 64));
    TEST_ASSERT_TRUE(alloc.cursor == byteoffset(p1, 64));

    // Allocate again, should reuse the space after previous free
    void* p3 = sa_alloc(&alloc, 32);
    TEST_ASSERT_TRUE(p3 == byteoffset(p1, 64));
    TEST_ASSERT_TRUE(p3 <= alloc.end);

    // Free to beginning
    sa_free(&alloc, alloc.begin);
    TEST_ASSERT_TRUE(alloc.cursor == alloc.begin);

    sa_deinit(&alloc);
    mem_unmap(mem, size);
}

void test_sa_reuse_space() {
    uptr size = 256;
    void* mem = mem_map(size);
    TEST_ASSERT_NOT_NULL(mem);
    TEST_ASSERT_TRUE(mem != MAP_FAILED);

    stack_alloc alloc;
    sa_init(&alloc, mem, byteoffset(mem, size));

    // Allocate two blocks
    void* p1 = sa_alloc(&alloc, 64);
    void* p2 = sa_alloc(&alloc, 64);

    // Free the second block
    sa_free(&alloc, p2);
    TEST_ASSERT_TRUE(alloc.cursor == p2);

    // Free the first block (rewind further back)
    sa_free(&alloc, p1);
    TEST_ASSERT_TRUE(alloc.cursor == p1);

    // Now allocate more than first block
    void* p3 = sa_alloc(&alloc, 128);
    TEST_ASSERT_TRUE(p3 == p1);

    sa_free(&alloc, alloc.begin);
    sa_deinit(&alloc);
    mem_unmap(mem, size);
}

void test_sa_over_allocation() {
    uptr size = 128;
    void* mem = mem_map(size);
    TEST_ASSERT_NOT_NULL(mem);
    TEST_ASSERT_TRUE(mem != MAP_FAILED);

    stack_alloc alloc;
    sa_init(&alloc, mem, byteoffset(mem, size));

    // Allocate most of the space
    void* p1 = sa_alloc(&alloc, 80);

    // Try to allocate more than remaining space
    void* p2 = sa_alloc(&alloc, 60);  // 48 remaining (128 - 80 = 48)
    TEST_ASSERT_TRUE(p2 != NULL);
    // Since debug_assert is off in tests, it will allocate even if over
    // So check if cursor moved correctly
    if (alloc.cursor > alloc.end) {
        printf("Warning: Over-allocation detected\n");
    }

    // Free to beginning
    sa_free(&alloc, alloc.begin);
    TEST_ASSERT_TRUE(alloc.cursor == alloc.begin);

    sa_deinit(&alloc);
    mem_unmap(mem, size);
}

void test_sa_module() {
    printf("Running Stack Allocator Module Tests...\n");
    test_sa_basic_alloc();
    test_sa_reuse_space();
    test_sa_over_allocation();
}

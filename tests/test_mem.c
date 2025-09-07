// Tests for memory module
#include "test_framework.h"
#include "../src/mem.h"
#include "../src/assert.h"
#include <sys/mman.h>
#include <stdio.h>

void test_mem_map_small_block(test_context* t) {
    uptr size = 1024;
    void* p = mem_map(size);
    TEST_ASSERT_NOT_NULL(t, p);
    TEST_ASSERT_TRUE(t, p != MAP_FAILED);

    // Write a byte to verify it's writable
    if (p != MAP_FAILED) {
        *(u8*)p = (u8)'X';
        TEST_ASSERT_EQUAL(t, *(u8*)p, (u8)'X');
    }

    mem_unmap(p, size);
}

void test_mem_map_zero_size(test_context* t) {
    uptr size = 0;
    void* p = mem_map(size);
    TEST_ASSERT_NOT_NULL(t, p);
    TEST_ASSERT_TRUE(t, p != MAP_FAILED);
    // Note: mmap with size 0 is allowed on some systems, but unmap may fail
    // For now, just attempt to unmap
    mem_unmap(p, size);
}

void test_mem_map_large_block(test_context* t) {
    uptr size = 1024 * 1024 * 10; // 10MB, should succeed if RAM allows
    void* p = mem_map(size);
    TEST_ASSERT_NOT_NULL(t, p);
    TEST_ASSERT_TRUE(t, p != MAP_FAILED);

    if (p != MAP_FAILED) {
        // Write to beginning and end to verify allocation
        *(u8*)p = (u8)'A';
        TEST_ASSERT_EQUAL(t, *(u8*)p, (u8)'A');

        *(u8*)byteoffset(p, size - 1) = (u8)'Z';
        TEST_ASSERT_EQUAL(t, *(u8*)byteoffset(p, size - 1), (u8)'Z');
    }

    mem_unmap(p, size);
}

void test_mem_module(test_context* t) {
    printf("Running Memory Module Tests...\n");
    test_mem_map_small_block(t);
    test_mem_map_zero_size(t);
    test_mem_map_large_block(t);
}

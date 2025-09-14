// Tests for memory module
#include "test_framework.h"
#include "../src/mem.h"
#include "../src/assert.h"
#include "../src/print.h"
#include "../src/file.h"
#include <stdio.h>

static void test_mem_map_small_block(test_context* t) {
    uptr size = 1024;
    void* p = mem_map(size);
    TEST_ASSERT_NOT_NULL(t, p);

    // Write a byte to verify it's writable
    *(u8*)p = (u8)'X';
    TEST_ASSERT_EQUAL(t, *(u8*)p, (u8)'X');

    mem_unmap(p, size);
}



static void test_mem_map_large_block(test_context* t) {
    uptr size = 1024 * 1024 * 10; // 10MB, should succeed if RAM allows
    void* p = mem_map(size);
    TEST_ASSERT_NOT_NULL(t, p);

    // Write to beginning and end to verify allocation
    *(u8*)p = (u8)'A';
    TEST_ASSERT_EQUAL(t, *(u8*)p, (u8)'A');

    *(u8*)byteoffset(p, size - 1) = (u8)'Z';
    TEST_ASSERT_EQUAL(t, *(u8*)byteoffset(p, size - 1), (u8)'Z');

    mem_unmap(p, size);
}

void test_mem_module(test_context* t) {
    print_string(file_get_stdout(), "Running Memory Module Tests...\n");
    test_mem_map_small_block(t);
    test_mem_map_large_block(t);
}

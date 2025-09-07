// Tests for stack allocator module
#include "test_framework.h"
#include "../src/mem.h"
#include "../src/stack_alloc.h"
#include "../src/primitive.h"
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

static void test_sa_move_tail(test_context* t) {
    uptr size = 1024;
    void* mem = mem_map(size);
    TEST_ASSERT_NOT_NULL(t, mem);

    stack_alloc alloc;
    sa_init(&alloc, mem, byteoffset(mem, size));

    // Allocate blocks for slice (0 to 100 inclusive = 101 elements)
    void* p1 = sa_alloc(&alloc, 101 * sizeof(i32));
    void* p2 = sa_alloc(&alloc, 32);  // small buffer after

    // Fill with test data (0 to 100)
    i32* slice = (i32*)p1;
    for (i32 i = 0; i <= 100; i++) {
        slice[i] = i;
    }
    *(i32*)p2 = 999;  // marker value

    // Move the tail from p1 to a new position
    void* new_start = byteoffset(mem, 128);
    sa_move_tail(&alloc, p1, new_start);

    // Check that cursor is updated (101 * 4 + 32 bytes)
    TEST_ASSERT_TRUE(t, alloc.cursor == byteoffset(new_start, 101 * sizeof(i32) + 32));

    // Check that position changed
    TEST_ASSERT_FALSE(t, (void*)&((i32*)new_start)[0] == p1);

    // Check that the moved slice contains 0 to 100
    i32* moved_slice = (i32*)new_start;
    for (i32 i = 0; i <= 100; i++) {
        TEST_ASSERT_TRUE(t, moved_slice[i] == i);
    }

    // Check that the data after the slice is also moved correctly
    TEST_ASSERT_TRUE(t, ((i32*)byteoffset(new_start, 101 * sizeof(i32)))[0] == 999);

    sa_free(&alloc, alloc.begin);
    sa_deinit(&alloc);
    mem_unmap(mem, size);
}

static void test_sa_move(test_context* t) {
    uptr size = 1024;
    void* mem = mem_map(size);
    TEST_ASSERT_NOT_NULL(t, mem);

    stack_alloc alloc;
    sa_init(&alloc, mem, byteoffset(mem, size));

    // Test 1: Move a block - cursor should never be updated
    void* p1 = sa_alloc(&alloc, 100 * sizeof(i32));  // 400 bytes, cursor now at mem+400

    // Fill with test data 0-99
    i32* slice = (i32*)p1;
    for (i32 i = 0; i < 100; i++) {
        slice[i] = i;
    }

    // Move the entire block to a new position
    uptr move_size = 50 * sizeof(i32);  // 200 bytes (first half of p1)
    void* new_position = byteoffset(mem, 600);
    void* original_cursor = alloc.cursor;  // should be mem+400
    sa_move(&alloc, p1, new_position, move_size);

    // Cursor should remain unchanged in sa_move (unlike sa_move_tail)
    TEST_ASSERT_TRUE(t, alloc.cursor == original_cursor);

    // Check moved data is correct
    i32* moved_slice = (i32*)new_position;
    for (i32 i = 0; i < 50; i++) {
        TEST_ASSERT_TRUE(t, moved_slice[i] == i);
    }

    // Reset for next test
    sa_free(&alloc, alloc.begin);

    // Test 2: Internal block move (should also not update cursor)
    void* base_block = sa_alloc(&alloc, 200 * sizeof(i32));  // Large block: 800 bytes
    void* small_block = sa_alloc(&alloc, 32);               // Small block: 32 bytes

    original_cursor = alloc.cursor;  // Should be at mem+832

    // Fill base block with test data 0-199
    slice = (i32*)base_block;
    for (i32 i = 0; i < 200; i++) {
        slice[i] = i;
    }
    *(i32*)small_block = 999;

    // Move middle portion (internal block move)
    move_size = 30 * sizeof(i32);  // 120 bytes, elements 50-79
    void* from_ptr = byteoffset(base_block, 50 * sizeof(i32));
    void* to_ptr = byteoffset(mem, 400);
    sa_move(&alloc, from_ptr, to_ptr, move_size);

    // Cursor should remain unchanged (sa_move never updates cursor)
    TEST_ASSERT_TRUE(t, alloc.cursor == original_cursor);

    // Check moved data is correct
    i32* internal_moved = (i32*)to_ptr;
    for (i32 i = 0; i < 30; i++) {
        TEST_ASSERT_TRUE(t, internal_moved[i] == i + 50);  // Was elements 50-79
    }

    // Check that other data is still intact
    TEST_ASSERT_TRUE(t, slice[25] == 25);  // Before moved region
    TEST_ASSERT_TRUE(t, slice[150] == 150);  // After moved region
    TEST_ASSERT_TRUE(t, *(i32*)small_block == 999);  // Small block preserved

    sa_free(&alloc, alloc.begin);
    sa_deinit(&alloc);
    mem_unmap(mem, size);
}

void test_sa_module(test_context* t) {
    printf("Running Stack Allocator Module Tests...\n");
    test_sa_basic_alloc(t);
    test_sa_reuse_space(t);
    test_sa_move_tail(t);
    test_sa_move(t);
}

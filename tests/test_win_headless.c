// Tests for win_headless module
#include "test_framework.h"
#include "../src/window/win_headless.h"
#include "../src/window/win.h"
#include "../src/stack_alloc.h"
#include "../src/mem.h"
#include "../src/primitive.h"
#include <stdio.h>

static void test_win_headless_basic_init_deinit(test_context* t) {
    printf("Testing basic win_headless init/deinit...\n");

    uptr size = 64 * 1024; // 64KB should be sufficient for basic operations
    void* mem = mem_map(size);
    TEST_ASSERT_NOT_NULL(t, mem);

    stack_alloc alloc;
    sa_init(&alloc, mem, byteoffset(mem, size));

    win_headless* win = win_headless_init(&alloc);
    TEST_ASSERT_NOT_NULL(t, win);
    TEST_ASSERT_NOT_NULL(t, win_headless_end(win));
    TEST_ASSERT_EQUAL(t, win_headless_end(win), alloc.cursor); // Should point to cursor after init

    win_headless_deinit(&alloc, win);
    // Test completes without crash - this is the main objective

    mem_unmap(mem, size);
    printf("Basic init/deinit test passed\n");
}

static void test_win_headless_window_lifecycle(test_context* t) {
    printf("Testing win_headless window lifecycle...\n");

    uptr size = 1024 * 1024; // 1MB for window and buffer
    void* mem = mem_map(size);
    TEST_ASSERT_NOT_NULL(t, mem);

    stack_alloc alloc;
    sa_init(&alloc, mem, byteoffset(mem, size));

    win_headless* win = win_headless_init(&alloc);
    TEST_ASSERT_NOT_NULL(t, win);

    // Check buffer access before window creation (should be NULL since no window)
    win_buffer buffer = win_headless_get_pixel_buffer(win);
    TEST_ASSERT_NULL(t, buffer.begin);
    TEST_ASSERT_NULL(t, buffer.end);

    // Open a small window for testing
    const char* title = "Test Window";
    i32 result = win_headless_open_window(win, title, 200, 150, &alloc);
    TEST_ASSERT_EQUAL(t, result, 1); // Should succeed

    // Check buffer after opening
    buffer = win_headless_get_pixel_buffer(win);
    TEST_ASSERT_NOT_NULL(t, buffer.begin);
    TEST_ASSERT_NOT_NULL(t, buffer.end);
    TEST_ASSERT_TRUE(t, buffer.begin < buffer.end); // Valid range

    // Test writing to buffer
    u32* pixels = (u32*)buffer.begin;
    pixels[0] = 0xFF0000FF; // Red
    if ((uptr)buffer.end - (uptr)buffer.begin > 100) {
        pixels[100] = 0x00FF00FF; // Green
    }

    // Present (no-op in headless)
    win_headless_present(win);

    // Close window
    win_headless_close_window(win);

    // Check buffer after close (should be NULL again)
    buffer = win_headless_get_pixel_buffer(win);
    TEST_ASSERT_NULL(t, buffer.begin);
    TEST_ASSERT_NULL(t, buffer.end);

    // Cleanup
    win_headless_deinit(&alloc, win);
    mem_unmap(mem, size);

    printf("Window lifecycle test passed\n");
}

static void test_win_headless_pixel_buffer_access(test_context* t) {
    printf("Testing pixel buffer access...\n");

    uptr size = 1024 * 1024; // 1MB for testing
    void* mem = mem_map(size);
    TEST_ASSERT_NOT_NULL(t, mem);

    stack_alloc alloc;
    sa_init(&alloc, mem, byteoffset(mem, size));

    win_headless* win = win_headless_init(&alloc);
    TEST_ASSERT_NOT_NULL(t, win);

    // Test buffer access before window creation
    win_buffer buffer = win_headless_get_pixel_buffer(win);
    TEST_ASSERT_NULL(t, buffer.begin);
    TEST_ASSERT_NULL(t, buffer.end);

    // Open window and test buffer again
    i32 result = win_headless_open_window(win, "Buffer Test", 100, 100, &alloc);
    TEST_ASSERT_EQUAL(t, result, 1);
    buffer = win_headless_get_pixel_buffer(win);
    TEST_ASSERT_NOT_NULL(t, buffer.begin);
    TEST_ASSERT_NOT_NULL(t, buffer.end);

    // Verify buffer size (100*100*4 bytes)
    uptr expected_size = 100UL * 100UL * 4UL;
    TEST_ASSERT_EQUAL(t, (uptr)buffer.end - (uptr)buffer.begin, expected_size);

    // Test writing beyond buffer (should not crash, but not tested here)
    u32* pixels = (u32*)buffer.begin;
    pixels[9999] = 0xFFFF00FF; // Yellow, last pixel

    // Close and verify buffer changes
    win_headless_close_window(win);
    buffer = win_headless_get_pixel_buffer(win);
    TEST_ASSERT_NULL(t, buffer.begin);
    TEST_ASSERT_NULL(t, buffer.end);

    win_headless_deinit(&alloc, win);
    mem_unmap(mem, size);

    printf("Pixel buffer access test passed\n");
}

static void test_win_headless_memory_boundaries(test_context* t) {
    printf("Testing memory boundaries...\n");

    uptr size = 512 * 1024; // 512KB for testing boundaries
    void* mem = mem_map(size);
    TEST_ASSERT_NOT_NULL(t, mem);

    stack_alloc alloc;
    sa_init(&alloc, mem, byteoffset(mem, size));

    win_headless* win = win_headless_init(&alloc);
    TEST_ASSERT_NOT_NULL(t, win);

    // Check initial memory boundary
    void* initial_end = win_headless_end(win);
    TEST_ASSERT_TRUE(t, initial_end == alloc.cursor);

    // Allocate space by opening window
    const char* title = "Boundary Test";
    i32 result = win_headless_open_window(win, title, 50, 50, &alloc);
    TEST_ASSERT_EQUAL(t, result, 1);

    // Check updated boundary
    void* after_open_end = win_headless_end(win);
    TEST_ASSERT_TRUE(t, after_open_end > initial_end);
    TEST_ASSERT_TRUE(t, after_open_end <= alloc.end);

    // Close window and check boundary reset
    win_headless_close_window(win);
    void* after_close_end = win_headless_end(win);
    TEST_ASSERT_TRUE(t, after_close_end >= initial_end);

    win_headless_deinit(&alloc, win);
    mem_unmap(mem, size);

    printf("Memory boundaries test passed\n");
}

static void test_win_headless_event_polling(test_context* t) {
    printf("Testing event polling...\n");

    uptr size = 128 * 1024; // 128KB for minimal test
    void* mem = mem_map(size);
    TEST_ASSERT_NOT_NULL(t, mem);

    stack_alloc alloc;
    sa_init(&alloc, mem, byteoffset(mem, size));

    win_headless* win = win_headless_init(&alloc);
    TEST_ASSERT_NOT_NULL(t, win);

    // Poll events (should return empty list pointing to cursor)
    win_event* events = win_headless_poll_events(win, &alloc);
    TEST_ASSERT_NOT_NULL(t, events);

    // In headless mode, events should point to alloc cursor (empty)
    TEST_ASSERT_EQUAL(t, (void*)events, alloc.cursor);

    win_headless_deinit(&alloc, win);
    mem_unmap(mem, size);

    printf("Event polling test passed\n");
}

void test_win_headless_module(test_context* t) {
    printf("Running win_headless Module Tests...\n");
    test_win_headless_basic_init_deinit(t);
    test_win_headless_window_lifecycle(t);
    test_win_headless_pixel_buffer_access(t);
    test_win_headless_memory_boundaries(t);
    test_win_headless_event_polling(t);
    printf("win_headless tests completed\n");
}

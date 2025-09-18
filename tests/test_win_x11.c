// Tests for win_x11 module
#include "test_framework.h"
#include "../src/window/win_x11.h"
#include "../src/window/win.h"
#include "../src/stack_alloc.h"
#include "../src/mem.h"
#include "../src/primitive.h"
#include "../src/print.h"
#include "../src/file.h"

static void test_win_x11_basic_init_deinit(test_context* t) {
    print_string(file_stdout(), STR_SPAN("Testing basic win_x11 init/deinit...\n"));

    uptr size = 64 * 1024; // 64KB should be sufficient for basic operations
    void* mem = mem_map(size);
    TEST_ASSERT_NOT_NULL(t, mem);

    stack_alloc alloc;
    sa_init(&alloc, mem, byteoffset(mem, size));

    win_x11* win = win_x11_init(&alloc);
    TEST_ASSERT_NOT_NULL(t, win);
    TEST_ASSERT_NOT_NULL(t, win_x11_end(win));

    win_x11_deinit(&alloc, win);
    // Test completes without crash - this is the main objective

    mem_unmap(mem, size);
    print_string(file_stdout(), STR_SPAN("Basic init/deinit test passed\n"));
}

static void test_win_x11_window_lifecycle(test_context* t) {
    print_string(file_stdout(), STR_SPAN("Testing win_x11 window lifecycle...\n"));

    uptr size = 1024 * 1024; // 1MB for window and buffer
    void* mem = mem_map(size);
    TEST_ASSERT_NOT_NULL(t, mem);

    stack_alloc alloc;
    sa_init(&alloc, mem, byteoffset(mem, size));

    win_x11* win = win_x11_init(&alloc);
    TEST_ASSERT_NOT_NULL(t, win);

    // Open a small window for testing
    const char* title = "Test Window";
    i32 result = win_x11_open_window(win, title, 200, 150, &alloc);
    TEST_ASSERT_EQUAL(t, result, 1); // Should succeed

    // Get pixel buffer and modify some pixels
    win_buffer buffer = win_x11_get_pixel_buffer(win);
    if (buffer.begin != 0 && buffer.end != 0) {
        // Set first pixel to red (0xFF0000FF in RGBA)
        u32* pixels = (u32*)buffer.begin;
        pixels[0] = 0xFF0000FF;

        // Set a few more pixels at different locations
        if ((uptr)buffer.end - (uptr)buffer.begin > 100) {
            pixels[100] = 0x00FF00FF; // Green
        }
        if ((uptr)buffer.end - (uptr)buffer.begin > 1000) {
            pixels[1000] = 0x0000FFFF; // Blue
        }

        // Present the changes (may not be visible in headless environment, but shouldn't crash)
        win_x11_present(win);
    }

    // Close window
    win_x11_close_window(win);

    // Cleanup
    win_x11_deinit(&alloc, win);
    mem_unmap(mem, size);

    print_string(file_stdout(), STR_SPAN("Window lifecycle test passed\n"));
}

static void test_win_x11_pixel_buffer_access(test_context* t) {
    print_string(file_stdout(), STR_SPAN("Testing pixel buffer access...\n"));

    uptr size = 1024 * 1024; // 1MB for testing
    void* mem = mem_map(size);
    TEST_ASSERT_NOT_NULL(t, mem);

    stack_alloc alloc;
    sa_init(&alloc, mem, byteoffset(mem, size));

    win_x11* win = win_x11_init(&alloc);
    TEST_ASSERT_NOT_NULL(t, win);

    // Test buffer access before window creation
    win_buffer buffer = win_x11_get_pixel_buffer(win);
    TEST_ASSERT_NOT_NULL(t, buffer.begin); // Initially points to alloc cursor
    TEST_ASSERT_NOT_NULL(t, buffer.end);   // Initially points to alloc cursor

    // Open window and test buffer again
    i32 result = win_x11_open_window(win, "Buffer Test", 100, 100, &alloc);
    if (result == 1) {
        buffer = win_x11_get_pixel_buffer(win);
        TEST_ASSERT_NOT_NULL(t, buffer.begin);
        TEST_ASSERT_NOT_NULL(t, buffer.end);
        TEST_ASSERT_TRUE(t, buffer.begin < buffer.end); // Valid buffer range
    }

    // Close and verify buffer changes (window.destroyed, but buffer references remain)
    win_x11_close_window(win);
    buffer = win_x11_get_pixel_buffer(win);
    TEST_ASSERT_NOT_NULL(t, buffer.begin); // Points to alloc cursor after dealloc

    win_x11_deinit(&alloc, win);
    mem_unmap(mem, size);

    print_string(file_stdout(), STR_SPAN("Pixel buffer access test passed\n"));
}

void test_win_x11_module(test_context* t) {
    print_string(file_stdout(), STR_SPAN("Registering win_x11 Module Tests...\n"));
    REGISTER_TEST(t, "win_x11_basic_init_deinit", test_win_x11_basic_init_deinit);
    REGISTER_TEST(t, "win_x11_window_lifecycle", test_win_x11_window_lifecycle);
    REGISTER_TEST(t, "win_x11_pixel_buffer_access", test_win_x11_pixel_buffer_access);
}

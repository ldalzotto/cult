#include <stdio.h>
#include "primitive.h"
#include "./mem.h"
#include "./stack_alloc.h"
#include "./window/win_x11.h"
#include "./assert.h"

static void memory_test() {
    // Simple test for stack_alloc
    uptr size = 1024;
    void* p = mem_map(size);
    
    stack_alloc alloc;
    sa_init(&alloc, p, byteoffset(p, size));

    // Allocate 64 bytes
    void* ptr1 = sa_alloc(&alloc, 64);
    printf("Allocated 64 bytes at address: %p\n", ptr1);

    // Try to allocate more than remaining space (200 - 64 = 136, allocate 160)
    void* ptr2 = sa_alloc(&alloc, 160);
    printf("Large allocation (160 bytes) success as expected: %p\n", ptr2);

    // Reset to beginning
    sa_free(&alloc, p);
    printf("Reset allocation pointer to beginning\n");

    // Allocate again, should reuse space
    void* ptr3 = sa_alloc(&alloc, 32);
    if (ptr3 == p) {
        printf("Allocation reused space correctly after reset\n");
    } else {
        printf("Allocation did not reuse space as expected\n");
    }

    sa_free(&alloc, p);

    sa_deinit(&alloc);

    mem_unmap(p, size);
}

i32 main() {
    // Test the primitive definitions
    i32 signed_32bit = -12345;
    u32 unsigned_32bit = 12345;

    printf("Hello, World!\n");
    printf("i32 value: %d\n", signed_32bit);
    printf("u32 value: %u\n", unsigned_32bit);

    memory_test();

    // Example usage of win_x11.h
    // Initialize stack allocator for window management
    uptr win_mem_size = 4096 * 1024; // 4KB should be sufficient for window context
    void* win_mem = mem_map(win_mem_size);

    stack_alloc win_alloc;
    sa_init(&win_alloc, win_mem, byteoffset(win_mem, win_mem_size));

    // Initialize X11 window context
    win_x11* win_ctx = win_x11_init(&win_alloc);
    if (win_ctx == NULL) {
        printf("Failed to initialize X11 window context\n");
        mem_unmap(win_mem, win_mem_size);
        return 1;
    }

    // Open a window
    i32 window_created = win_x11_open_window(win_ctx, "X11 Window Example", 200, 200, &win_alloc);
    if (window_created == 0) {
        printf("Failed to create window\n");
        win_x11_deinit(&win_alloc, win_ctx);
        mem_unmap(win_mem, win_mem_size);
        return 1;
    }

    printf("Window created successfully. Press Ctrl+C to exit.\n");

    // Simple event polling loop
    // In a real application, you'd have proper event handling
    while (1) {
        u8 should_exit = 0;
        win_event* event_begin = win_x11_poll_events(win_ctx, &win_alloc);
        win_event* event_end = win_alloc.cursor;
        for (win_event* event = event_begin; event < event_end;++event) {
            // Simple event handling - just print for demonstration
            printf("Received event type: %d\n", event->type);
            if (event->type == 2) {
                should_exit = 1;
            }
        }

        // Free the event memory if it was allocated
        sa_free(&win_alloc, event_begin);

        win_buffer buffer = win_x11_get_pixel_buffer(win_ctx);
        debug_assert(bytesize(buffer.begin, buffer.end) % sizeof(i32) == 0);
        for (i32* x = buffer.begin; x < (i32*)buffer.end; ++x) {
            *x = 100;
        }

        win_x11_present(win_ctx);

        if (should_exit) {
            break;
        }
    }

    // Close the window
    win_x11_close_window(win_ctx);

    // Deinitialize
    win_x11_deinit(&win_alloc, win_ctx);

    // Clean up memory
    sa_deinit(&win_alloc);
    mem_unmap(win_mem, win_mem_size);

    return 0;
}

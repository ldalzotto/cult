#include "snake.h"
#include "primitive.h"
#include "mem.h"
#include "stack_alloc.h"
#include "window/win_x11.h"
#include "assert.h"
#include "print.h"
#include "file.h"
#include "system_time.h"
#include "time.h"

i32 main(void) {
    // Initialize stack allocator for window management
    uptr win_mem_size = 4096 * 1024; // 4MB for window context
    void* win_mem = mem_map(win_mem_size);

    stack_alloc win_alloc;
    sa_init(&win_alloc, win_mem, byteoffset(win_mem, win_mem_size));

    // Initialize X11 window context
    win_x11* win_ctx = win_x11_init(&win_alloc);
    if (win_ctx == 0) {
        print_string(file_stderr(), STRING("Failed to initialize X11 window context\n"));
        mem_unmap(win_mem, win_mem_size);
        return 1;
    }

    // Open a window
    const i32 WIDTH = 200;
    const i32 HEIGHT = 200;
    i32 window_created = win_x11_open_window(win_ctx, "Snake Game", WIDTH, HEIGHT, &win_alloc);
    if (window_created == 0) {
        print_string(file_stderr(), STRING("Failed to create window\n"));
        win_x11_deinit(&win_alloc, win_ctx);
        mem_unmap(win_mem, win_mem_size);
        return 1;
    }

    print_string(file_stdout(), STRING("Window created successfully. Press Ctrl+C to exit.\n"));

    // Allocate snake
    snake* s = snake_init(&win_alloc);

    // ---------------- FPS TICKER ----------------
    fps_ticker ticker;
    const u64 preferred_frame_us = 16666; // ~60 FPS
    u64 start_time = sys_time_us();
    fps_ticker_init(&ticker, preferred_frame_us, start_time);

    // Main loop
    while (1) {
        u64 now = sys_time_us();
        u32 frames_to_process = fps_ticker_update(&ticker, now);

        if (frames_to_process == 0) {
            // Not enough time elapsed for the next frame
            continue;
        }

        // Poll events (for now, only check close)
        win_event* event_begin = win_x11_poll_events(win_ctx, &win_alloc);
        win_event* event_end = win_alloc.cursor;
        u8 should_exit = 0;

        for (win_event* event = event_begin; event < event_end; ++event) {
            if (event->type == WIN_EVENT_TYPE_RELEASED) { // Close event
                should_exit = 1;
            }
        }
        sa_free(&win_alloc, event_begin);

        // Update snake
        snake_update(s, ticker.preferred_frame_us, &win_alloc);

        // Get buffer and render
        win_buffer buffer = win_x11_get_pixel_buffer(win_ctx);
        debug_assert(bytesize(buffer.begin, buffer.end) % sizeof(i32) == 0);

        // Clear buffer to black
        for (i32* pixel = (i32*)buffer.begin; pixel < (i32*)buffer.end; ++pixel) {
            *pixel = 0x00000000; // Black
        }

        // Draw snake head
        i32 grid_width = snake_get_grid_width(s);
        i32 grid_height = snake_get_grid_height(s);
        i32 cell_size_x = WIDTH / grid_width;
        i32 cell_size_y = HEIGHT / grid_height;
        i32 start_x = snake_get_head_x(s) * cell_size_x;
        i32 start_y = snake_get_head_y(s) * cell_size_y;
        for (i32 dy = 0; dy < cell_size_y; ++dy) {
            for (i32 dx = 0; dx < cell_size_x; ++dx) {
                i32 px = start_x + dx;
                i32 py = start_y + dy;
                if (px >= 0 && px < WIDTH && py >= 0 && py < HEIGHT) {
                    ((i32*)buffer.begin)[py * WIDTH + px] = 0x00FFFFFF; // White
                }
            }
        }

        win_x11_present(win_ctx);

        if (should_exit) {
            break;
        }
    }

    // Deinit snake
    snake_deinit(s, &win_alloc);

    // Close window
    win_x11_close_window(win_ctx);

    // Deinitialize
    win_x11_deinit(&win_alloc, win_ctx);
    sa_deinit(&win_alloc);
    mem_unmap(win_mem, win_mem_size);

    return 0;
}

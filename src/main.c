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
    i32 window_created = win_x11_open_window(win_ctx, "X11 Window Example", 200, 200, &win_alloc);
    if (window_created == 0) {
        print_string(file_stderr(), STRING("Failed to create window\n"));
        win_x11_deinit(&win_alloc, win_ctx);
        mem_unmap(win_mem, win_mem_size);
        return 1;
    }

    print_string(file_stdout(), STRING("Window created successfully. Press Ctrl+C to exit.\n"));

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

        // Poll events
        win_event* event_begin = win_x11_poll_events(win_ctx, &win_alloc);
        win_event* event_end = win_alloc.cursor;
        u8 should_exit = 0;

        for (win_event* event = event_begin; event < event_end; ++event) {
            print_format(file_stdout(), STRING("Received event type: %d\n"), event->type);
            if (event->type == 2) { // Close event
                should_exit = 1;
            }
        }
        sa_free(&win_alloc, event_begin);

        win_buffer buffer = win_x11_get_pixel_buffer(win_ctx);
        debug_assert(bytesize(buffer.begin, buffer.end) % sizeof(i32) == 0);
        for (i32* x = buffer.begin; x < (i32*)buffer.end; ++x) {
            *x = *x+1; // simple fill
        }
        win_x11_present(win_ctx);

        if (should_exit) {
            break;
        }
    }

    // Close window
    win_x11_close_window(win_ctx);

    // Deinitialize
    win_x11_deinit(&win_alloc, win_ctx);
    sa_deinit(&win_alloc);
    mem_unmap(win_mem, win_mem_size);

    return 0;
}

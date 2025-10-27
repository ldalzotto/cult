#include "snake.h"
#include "snake_render.h"
#include "primitive.h"
#include "mem.h"
#include "stack_alloc.h"
#include "window/win_x11.h"
#include "assert.h"
#include "print.h"
#include "file.h"
#include "system_time.h"
#include "time.h"
#include "thread.h"

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
            u64 to_wait_us = fps_ticker_estimated_next_frame(ticker);
            thread_current_sleep_until_us(to_wait_us);
            continue;
        }

        // Poll events (for now, only check close)
        win_event* event_begin = win_x11_poll_events(win_ctx, &win_alloc);
        win_event* event_end = win_alloc.cursor;
        u8 should_exit = 0;

        snake_input input = {.down = 0, .left = 0,.right = 0,.up = 0};

        for (win_event* event = event_begin; event < event_end; ++event) {
            if (event->type == WIN_EVENT_TYPE_PRESSED) {
                switch (event->key) {
                    case WIN_KEY_LEFT: input.left = 1; break;
                    case WIN_KEY_RIGHT: input.right = 1; break;
                    case WIN_KEY_UP: input.up = 1; break;
                    case WIN_KEY_DOWN: input.down = 1; break;
                    default: break;
                }
            } else if (event->type == WIN_EVENT_TYPE_RELEASED) {
                switch (event->key) {
                    case WIN_KEY_LEFT: input.left = 0; break;
                    case WIN_KEY_RIGHT: input.right = 0; break;
                    case WIN_KEY_UP: input.up = 0; break;
                    case WIN_KEY_DOWN: input.down = 0; break;
                    default:
                        if (event->key == WIN_KEY_UNKNOWN) { // Close event
                            should_exit = 1;
                        }
                        break;
                }
            }
        }
        sa_free(&win_alloc, event_begin);

        // If the snake reports STOP, restart the game
        void* s_end_before = snake_end(s);
        snake_update_result upd = snake_update(s, input, ticker.preferred_frame_us, &win_alloc);
        if (snake_end(s) != s_end_before) {
            print_string(file_stdout(), STRING("Memory layout changed\n"));
            /*
                The memory layout changed.
                Nothing to do for now. Because nothing is allocated past the snake module.
            */
        }
        if (upd == SNAKE_UPDATE_STOP) {
            // Restart the game
            snake_deinit(s, &win_alloc);
            s = snake_init(&win_alloc);
            // Skip rendering this frame after restart
            continue;
        }

        // Get buffer and render
        win_buffer buffer = win_x11_get_pixel_buffer(win_ctx);
        debug_assert(bytesize(buffer.begin, buffer.end) % sizeof(i32) == 0);

        // Render snake to commands
        u32 command_count;
        draw_command* cmds = snake_render(s, WIDTH, HEIGHT, &command_count, &win_alloc);

        // Execute draw commands
        for (u32 i = 0; i < command_count; ++i) {
            draw_command cmd = cmds[i];
            if (cmd.type == DRAW_COMMAND_CLEAR_BACKGROUND) {
                // Clear buffer to the specified color
                for (i32* pixel = (i32*)buffer.begin; pixel < (i32*)buffer.end; ++pixel) {
                    *pixel = (i32)cmd.data.clear_bg.color;
                }
            } else if (cmd.type == DRAW_COMMAND_DRAW_RECTANGLE) {
                // Draw rectangle
                for (i32 dy = 0; dy < cmd.data.rect.h; ++dy) {
                    for (i32 dx = 0; dx < cmd.data.rect.w; ++dx) {
                        i32 px = cmd.data.rect.x + dx;
                        i32 py = cmd.data.rect.y + dy;
                        if (px >= 0 && px < WIDTH && py >= 0 && py < HEIGHT) {
                            ((i32*)buffer.begin)[py * WIDTH + px] = (i32)cmd.data.rect.color;
                        }
                    }
                }
            }
        }

        // Free the commands
        sa_free(&win_alloc, cmds);

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

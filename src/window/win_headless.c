/**
 * @file win_headless.c
 * @brief Implementation of headless window management abstraction
 *
 * This file implements the win_headless module without any external dependencies.
 * All display operations are simulated, providing a clean abstraction boundary
 * for user code in headless environments.
 *
 * Memory management is handled entirely through stack allocator functions.
 * The module provides thin wrappers around essential window operations:
 * - Simulated display connection management
 * - Simulated window creation and destruction
 * - Basic window properties setup
 */

#include <stdio.h>
#include "../assert.h"
#include "../stack_alloc.h"
#include "../primitive.h"
#include "win_headless.h"

/**
 * @brief Internal headless context structure
 *
 * Contains simulated state for headless operation. This structure is opaque
 * to user code to maintain clean separation between simulated and real
 * window management.
 */
struct win_headless {
    u8 is_initialized;     /**< Simulated display connection status */
    u8 is_window_open;     /**< Whether a simulated window is open */
    u32* pixel_buffer;       /**< Pixel buffer for direct writing (RGBA) */
    u32* pixel_buffer_end;   /**< End of pixel buffer for memory tracking */
    i32 buffer_width;        /**< Width of pixel buffer */
    i32 buffer_height;       /**< Height of pixel buffer */
    const char* window_title;/**< Stored window title for simulation */
};

/**
 * @brief Implementation of win_headless_init()
 *
 * Allocates win_headless struct from stack allocator and simulates
 * display connection initialization.
 */
win_headless* win_headless_init(stack_alloc* alloc) {
    // Allocate memory for win_headless struct
    win_headless* win = (win_headless*)sa_alloc(alloc, sizeof(win_headless));

    // Simulate successful display connection
    win->is_initialized = 1;
    win->is_window_open = 0;
    win->pixel_buffer = alloc->cursor;
    win->pixel_buffer_end = alloc->cursor;
    win->buffer_width = 0;
    win->buffer_height = 0;
    win->window_title = NULL;

    return win;
}

/**
 * @brief Implementation of win_headless_end()
 *
 * Returns address immediately after win_headless struct.
 * Used for stack allocator memory boundary tracking.
 */
void* win_headless_end(win_headless* win) {
    return win->pixel_buffer_end;
}

/**
 * @brief Implementation of win_headless_deinit()
 *
 * Safely shuts down simulated resources in correct order:
 * 1. Close any simulated windows
 * 2. Reset display connection status
 * 3. Free win_headless struct from stack allocator
 */
void win_headless_deinit(stack_alloc* alloc, win_headless* win) {
    debug_assert(alloc != NULL);
    debug_assert(win->is_initialized);
    
    // Close window if it exists
    win_headless_close_window(win);

    // Reset simulation state
    win->is_initialized = 0;

    // Free the win_headless struct
    sa_free(alloc, win);
}

/**
 * @brief Implementation of win_headless_open_window()
 *
 * Simulates window creation with specified title and dimensions:
 * - Sets window title for simulation
 * - Allocates pixel buffer based on dimensions
 * - Initializes buffer with white pixels
 */
i32 win_headless_open_window(win_headless* win, const char* title, i32 width, i32 height, stack_alloc* alloc) {
    debug_assert(win->is_initialized);
    debug_assert(title != NULL);
    debug_assert(width > 0 && height > 0);
    debug_assert(alloc->cursor == win_headless_end(win));

    // Close existing window if open
    if (win->is_window_open) {
        win_headless_close_window(win);
    }

    // Set window title (store reference, don't copy)
    win->window_title = title;

    // Allocate pixel buffer directly after win_headless struct for contiguous memory
    uptr buffer_size = (uptr)width * (uptr)height * 4;  // RGBA: 4 bytes per pixel
    win->pixel_buffer = (u32*)sa_alloc(alloc, buffer_size);
    win->pixel_buffer_end = alloc->cursor;

    // Initialize buffer with white pixels
    for (u32* pixel = win->pixel_buffer; pixel < win->pixel_buffer_end; ++pixel) {
        *pixel = 0xFFFFFFFF;  // White RGBA
    }

    // Store buffer dimensions
    win->buffer_width = width;
    win->buffer_height = height;
    win->is_window_open = 1;

    return 1;  // true - always succeeds in simulation
}

/**
 * @brief Implementation of win_headless_close_window()
 *
 * Safely destroys the current simulated window if one exists:
 * - Resets pixel buffer pointers
 * - Sets window status to closed
 * - Resets buffer dimensions
 * - No-op if no window is currently open
 */
void win_headless_close_window(win_headless* win) {
    debug_assert(win->is_initialized);

    if (win->is_window_open) {
        // Reset pixel buffer resources
        win->pixel_buffer = (u32*)byteoffset(win, sizeof(*win));
        win->pixel_buffer_end = (u32*)byteoffset(win, sizeof(*win));
        win->buffer_width = 0;
        win->buffer_height = 0;
        win->window_title = NULL;
        win->is_window_open = 0;
    }
}

/**
 * @brief Implementation of win_headless_poll_events()
 *
 * In headless mode, no real events exist, so always returns an empty event list.
 * This maintains API compatibility while simulating event polling.
 */
win_event* win_headless_poll_events(win_headless* win, stack_alloc* alloc) {
    debug_assert(alloc != NULL);
    debug_assert(win->is_initialized);

    // In headless mode, there are no events to poll
    // Return the current cursor position (empty event list)
    return (win_event*)alloc->cursor;
}

/**
 * @brief Implementation of win_headless_get_pixel_buffer()
 *
 * Returns the base address of the pixel buffer for direct pixel data access.
 * The buffer contains 32-bit RGBA pixels in row-major order.
 */
win_buffer win_headless_get_pixel_buffer(win_headless* win) {
    win_buffer buffer;
    if (win->is_window_open) {
        buffer.begin = win->pixel_buffer;
        buffer.end = win->pixel_buffer_end;
    } else {
        buffer.begin = NULL;
        buffer.end = NULL;
    }
    return buffer;
}

/**
 * @brief Implementation of win_headless_present()
 *
 * In headless mode, this is a no-op as there is no display to update.
 * Maintains API compatibility with win_x11.
 */
void win_headless_present(win_headless* win) {
    debug_assert(win->is_initialized);
    debug_assert(win->is_window_open);

    // No-op: nothing to display in headless mode
    // This function exists solely for API compatibility
}

/**
 * @file win_headless.h
 * @brief Thin abstraction layer for window management in headless mode
 *
 * This module provides a clean, minimal interface for creating and managing
 * simulated windows without any display dependencies. All memory is managed
 * through the stack allocator system. This is the headless counterpart to
 * win_x11.h, simulating window state for testing or server environments.
 */

#ifndef WIN_HEADLESS_H
#define WIN_HEADLESS_H

#include "../stack_alloc.h"
#include "./win.h"

/**
 * @brief Opaque handle to headless window context
 *
 * This struct manages simulated window state without external dependencies.
 * The internal structure is opaque.
 */
typedef struct win_headless win_headless;

win_event* win_headless_poll_events(win_headless* win, stack_alloc* alloc);

/**
 * @brief Initialize headless window context
 *
 * Allocates win_headless struct using the provided stack allocator.
 * Simulates connection to a display server.
 *
 * @param alloc Pointer to initialized stack allocator for memory management
 *
 * @return Pointer to initialized win_headless context on success, NULL on failure
 * @return NULL if memory allocation fails
 *
 * @pre alloc must be a valid stack allocator
 * @post Returns valid win_headless context or cleans up partial initialization
 */
win_headless* win_headless_init(stack_alloc* alloc);

/**
 * @brief Deinitialize headless window context and free resources
 *
 * Destroys any simulated windows and frees the win_headless struct
 * using stack allocator.
 *
 * @param alloc Pointer to stack allocator used for initial allocation
 * @param win Pointer to win_headless context to deinitialize
 *
 * @return void
 * @pre win must be valid win_headless context
 * @pre alloc must be same allocator used for initialization
 * @post win_headless struct is freed, windows destroyed
 */
void win_headless_deinit(stack_alloc* alloc, win_headless* win);

/**
 * @brief Get address of end of win_headless module
 *
 * Useful for stack allocator memory management to determine the boundary
 * after the win_headless struct for subsequent allocations.
 *
 * @param win Pointer to win_headless context
 *
 * @return Address immediately after the win_headless struct
 * @pre win must be valid win_headless context
 */
void* win_headless_end(win_headless* win);

/**
 * @brief Create and simulate a new window
 *
 * Simulates a window with specified title and dimensions.
 * Allocates pixel buffer for rendering.
 *
 * @param win Pointer to initialized win_headless context
 * @param title Window title string (stored internally)
 * @param width Window width in pixels (must be > 0)
 * @param height Window height in pixels (must be > 0)
 *
 * @return 1 (true) on success, 0 (false) on failure
 * @return 0 if memory allocation fails
 *
 * @pre win must be valid win_headless context
 * @pre title must not be NULL
 * @post If successful, simulated window is created with allocated buffer
 */
i32 win_headless_open_window(win_headless* win, const char* title, i32 width, i32 height, stack_alloc* alloc);

/**
 * @brief Close and destroy the current simulated window
 *
 * Destroys the simulated window if one exists and cleans up associated resources.
 * Safe to call multiple times or if no window exists.
 *
 * @param win Pointer to initialized win_headless context
 *
 * @return void
 * @pre win must be valid win_headless context
 * @post Window is destroyed, resources cleared
 */
void win_headless_close_window(win_headless* win);

/**
 * @brief Get pixel buffer base address for direct pixel data access
 *
 * Returns a pointer to the pixel buffer that can be written to directly.
 * The buffer contains 32-bit RGBA pixels in row-major order.
 *
 * @param win Pointer to initialized win_headless context
 *
 * @return Pointer to pixel buffer (uint32_t*), or NULL if no window is open
 * @pre win must be valid win_headless context
 * @pre A window must be open (call win_headless_open_window first)
 */
win_buffer win_headless_get_pixel_buffer(win_headless* win);

/**
 * @brief Present pixel buffer contents (no-op in headless mode)
 *
 * In headless mode, this function does nothing as there is no display
 * to update. It's included for API compatibility with win_x11.
 *
 * @param win Pointer to initialized win_headless context
 *
 * @return void
 * @pre win must be valid win_headless context
 * @pre A window must be open and have an initialized pixel buffer
 */
void win_headless_present(win_headless* win);


#endif /* WIN_HEADLESS_H */

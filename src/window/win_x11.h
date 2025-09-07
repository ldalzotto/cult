/**
 * @file win_x11.h
 * @brief Thin abstraction layer over X11 window management
 *
 * This module provides a clean, minimal interface for creating and managing
 * X11 windows while hiding all X11-specific headers and complexities.
 * All memory is managed through the stack allocator system.
 */

#ifndef WIN_X11_H
#define WIN_X11_H

#include "../primitive.h"
#include "../stack_alloc.h"

/**
 * @brief Opaque handle to X11 window context
 *
 * This struct manages Display connections, window handles, and screen information.
 * The internal structure is opaque to prevent X11 dependencies from leaking
 * into user code.
 */
typedef struct win_x11 win_x11;

/**
 * @brief Initialize X11 window context
 *
 * Opens connection to X display server and allocates win_x11 struct using
 * the provided stack allocator.
 *
 * @param alloc Pointer to initialized stack allocator for memory management
 *
 * @return Pointer to initialized win_x11 context on success, NULL on failure
 * @return NULL if X display cannot be opened or memory allocation fails
 *
 * @pre alloc must be a valid stack allocator
 * @post Returns valid win_x11 context or cleans up partial initialization
 */
win_x11* win_x11_init(stack_alloc* alloc);

/**
 * @brief Deinitialize X11 window context and free resources
 *
 * Closes X display connection, destroys any open windows, and frees
 * the win_x11 struct using stack allocator.
 *
 * @param alloc Pointer to stack allocator used for initial allocation
 * @param win Pointer to win_x11 context to deinitialize
 *
 * @return void
 * @pre win must be valid win_x11 context
 * @pre alloc must be same allocator used for initialization
 * @post win_x11 struct is freed, display closed, windows destroyed
 */
void win_x11_deinit(stack_alloc* alloc, win_x11* win);

/**
 * @brief Get address of end of win_x11 module
 *
 * Useful for stack allocator memory management to determine the boundary
 * after the win_x11 struct for subsequent allocations.
 *
 * @param win Pointer to win_x11 context
 *
 * @return Address immediately after the win_x11 struct
 * @pre win must be valid win_x11 context
 */
void* win_x11_end(win_x11* win);

/**
 * @brief Create and display a new window
 *
 * Creates a simple X11 window with specified title and dimensions,
 * centered on the screen. The window becomes immediately visible.
 *
 * @param win Pointer to initialized win_x11 context
 * @param title Window title string (copied internally)
 * @param width Window width in pixels (must be > 0)
 * @param height Window height in pixels (must be > 0)
 *
 * @return 1 (true) on success, 0 (false) on failure
 * @return 0 if X11 window creation fails
 *
 * @pre win must be valid win_x11 context
 * @pre title must not be NULL
 * @post If successful, window is created and mapped on screen
 */
i32 win_x11_open_window(win_x11* win, const char* title, i32 width, i32 height);

/**
 * @brief Close and destroy the current window
 *
 * Destroys the X11 window if one exists and cleans up associated resources.
 * Safe to call multiple times or if no window exists.
 *
 * @param win Pointer to initialized win_x11 context
 *
 * @return void
 * @pre win must be valid win_x11 context
 * @post Window is destroyed, resources cleared
 */
void win_x11_close_window(win_x11* win);


#endif /* WIN_X11_H */

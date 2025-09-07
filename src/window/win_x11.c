/**
 * @file win_x11.c
 * @brief Implementation of X11 window management abstraction
 *
 * This file implements the win_x11 module using X11 library functions.
 * All X11 headers and complexities are contained here, providing a clean
 * abstraction boundary for user code.
 *
 * Memory management is handled entirely through stack allocator functions.
 * The module provides thin wrappers around essential X11 window operations:
 * - Display connection management
 * - Window creation and destruction
 * - Basic window properties and event setup
 */

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <stdio.h>
#include "../assert.h"
#include "../stack_alloc.h"
#include "../primitive.h"
#include "win_x11.h"

/**
 * @brief Internal X11 context structure
 *
 * Contains all X11-specific state and handles. This structure is opaque
 * to user code to maintain clean separation between X11 dependencies
 * and application logic.
 */
struct win_x11 {
    Display* display;        /**< Connection to X server */
    Window window;           /**< X11 window handle (0 if no window) */
    i32 screen_num;          /**< Screen number for display */
    XImage* pixel_buffer_img;/**< X11 image for efficient presentation */
    u32* pixel_buffer;       /**< Pixel buffer for direct writing (RGBA) */
    u32* pixel_buffer_end;
    i32 buffer_width;        /**< Width of pixel buffer */
    i32 buffer_height;       /**< Height of pixel buffer */
};

/**
 * @brief Implementation of win_x11_init()
 *
 * Allocates win_x11 struct from stack allocator and opens X display connection.
 * Uses progressive initialization - cleans up on partial failure.
 */
win_x11* win_x11_init(stack_alloc* alloc) {
    // Allocate memory for win_x11 struct
    win_x11* win = (win_x11*)sa_alloc(alloc, sizeof(win_x11));

    // Open connection to X server
    win->display = XOpenDisplay(NULL);
    if (!win->display) {
        printf("win_x11: Cannot open display\n");
        sa_free(alloc, win);
        return alloc->cursor;
    }

    win->screen_num = (i32)DefaultScreen(win->display);
    win->window = 0;  // No window opened yet
    win->pixel_buffer = alloc->cursor;
    win->pixel_buffer_end = alloc->cursor;
    win->buffer_width = 0;
    win->buffer_height = 0;

    return win;
}

/**
 * @brief Implementation of win_x11_end()
 *
 * Returns address immediately after win_x11 struct.
 * Used for stack allocator memory boundary tracking.
 */
void* win_x11_end(win_x11* win) {
    return win->pixel_buffer_end;
}

/**
 * @brief Implementation of win_x11_deinit()
 *
 * Safely shuts down X11 resources in correct order:
 * 1. Close any open windows
 * 2. Close display connection
 * 3. Free win_x11 struct from stack allocator
 */
void win_x11_deinit(stack_alloc* alloc, win_x11* win) {
    debug_assert(alloc != NULL);
    debug_assert(win->display != NULL);
    
    // Close window if it exists
    win_x11_close_window(win);

    // Close the display connection if it exists
    if (win->display) {
        XCloseDisplay(win->display);
        win->display = NULL;
    }

    // Free the win_x11 struct
    sa_free(alloc, win);
}

/**
 * @brief Implementation of win_x11_open_window()
 *
 * Creates a simple window centered on screen with:
 * - White background, black border
 * - Window title from parameter
 * - Exposure and KeyPress event handling
 * - Auto-centering on current screen
 */
i32 win_x11_open_window(win_x11* win, const char* title, i32 width, i32 height, stack_alloc* alloc) {
    debug_assert(win->display != NULL);
    debug_assert(title != NULL);
    debug_assert(width > 0 && height > 0);
    debug_assert(alloc->cursor == win_x11_end(win));

    // Get screen dimensions
    Screen* screen = ScreenOfDisplay(win->display, win->screen_num);
    i32 screen_width = (i32)WidthOfScreen(screen);
    i32 screen_height = (i32)HeightOfScreen(screen);

    // Center the window on screen
    i32 x = (screen_width - width) / 2;
    i32 y = (screen_height - height) / 2;

    // Create the window
    win->window = XCreateSimpleWindow(win->display,
                                     RootWindow(win->display, win->screen_num),
                                     x, y, width, height, 1,
                                     BlackPixel(win->display, win->screen_num),
                                     WhitePixel(win->display, win->screen_num));

    if (!win->window) {
        printf("win_x11: Cannot create window\n");
        return 0;  // false
    }

    // Set window title
    XStoreName(win->display, win->window, title);

    // Select input events
    XSelectInput(win->display, win->window, ExposureMask | KeyPressMask);

    // Make window visible
    XMapWindow(win->display, win->window);

    // Allocate pixel buffer (RGBA format - 4 bytes per pixel)
    uptr buffer_size = (uptr)width * (uptr)height * 4;

    // Allocate directly after win_x11 struct for contiguous memory
    win->pixel_buffer = (u32*)sa_alloc(alloc, buffer_size);
    win->pixel_buffer_end = alloc->cursor;

    // Initialize buffer with white pixels
    uptr pixel_count = (uptr)width * (uptr)height;
    for (uptr i = 0; i < pixel_count; ++i) {
        win->pixel_buffer[i] = 0xFFFFFFFF;  // White RGBA
    }

    // Create XImage for efficient presentation
    Visual* visual = DefaultVisual(win->display, win->screen_num);
    win->pixel_buffer_img = XCreateImage(win->display, visual, 24, ZPixmap, 0,
                                        (char*)win->pixel_buffer, width, height, 32, 0);
    if (!win->pixel_buffer_img) {
        printf("win_x11: Cannot create XImage\n");
        sa_free(alloc, win->pixel_buffer);
        win->pixel_buffer = NULL;
        XDestroyWindow(win->display, win->window);
        win->window = 0;
        XFlush(win->display);
        return 0;  // false
    }

    // Store buffer dimensions
    win->buffer_width = width;
    win->buffer_height = height;

    // Flush any pending requests
    XFlush(win->display);

    return 1;  // true
}

/**
 * @brief Implementation of win_x11_close_window()
 *
 * Safely destroys the current window if one exists:
 * - Calls XDestroyWindow() to clean up window resources
 * - Cleans up pixel buffer resources
 * - Sets window handle to 0 to indicate no window
 * - Flushes X requests to ensure destruction completes
 * - No-op if no window is currently open
 */
void win_x11_close_window(win_x11* win) {
    debug_assert(win->display != NULL);

    if (win->window) {
        // Clean up pixel buffer resources
        if (win->pixel_buffer_img) {
            win->pixel_buffer_img->data = NULL;
            XDestroyImage(win->pixel_buffer_img);
            win->pixel_buffer_img = NULL;
        }
        win->buffer_width = 0;
        win->buffer_height = 0;

        // Destroy the window
        XDestroyWindow(win->display, win->window);
        win->window = 0;
        XFlush(win->display);
    }
}

win_event* win_x11_poll_events(win_x11* win, stack_alloc* alloc) {
    debug_assert(alloc != NULL);
    debug_assert(win->display != NULL);

    win_event* events_start = (win_event*)alloc->cursor;

    // Poll all pending events using while loop
    while (XPending(win->display) > 0) {
        win_event* event = sa_alloc(alloc, sizeof(*event));
        debug_assert(event != NULL);
        XEvent xevent;
        XNextEvent(win->display, &xevent);
        event->type = xevent.type;
    }

    return events_start;
}

/**
 * @brief Implementation of win_x11_get_pixel_buffer()
 *
 * Returns the base address of the pixel buffer for direct pixel data access.
 * The buffer contains 32-bit RGBA pixels in row-major order.
 */
win_buffer win_x11_get_pixel_buffer(win_x11* win) {
    win_buffer buffer;
    buffer.begin = win->pixel_buffer;
    buffer.end = win->pixel_buffer_end;
    return buffer;
}

/**
 * @brief Implementation of win_x11_present()
 *
 * Updates the window display with the current contents of the pixel buffer.
 * Uses XPutImage to efficiently copy the pixel buffer to the window surface.
 */
void win_x11_present(win_x11* win) {
    debug_assert(win->display != NULL);
    debug_assert(win->pixel_buffer_img != NULL);
    debug_assert(win->window != 0);

    // Copy the pixel buffer image to the window
    XPutImage(win->display, win->window, DefaultGC(win->display, win->screen_num),
             win->pixel_buffer_img, 0, 0, 0, 0, win->buffer_width, win->buffer_height);

    // Flush to ensure the update is displayed immediately
    XFlush(win->display);
}

#ifndef SNAKE_RENDER_H
#define SNAKE_RENDER_H

#include "stack_alloc.h"
#include "snake_asset.h"

typedef struct snake snake;

/* Draw command API for snake rendering. */

typedef enum {
    DRAW_COMMAND_CLEAR_BACKGROUND,
    DRAW_COMMAND_DRAW_RECTANGLE,
    DRAW_COMMAND_DRAW_RECTANGLE_TEXTURED
} draw_command_type;

typedef struct draw_clear_background {
    u32 color; // RGBA
} draw_clear_background;

typedef struct draw_rectangle {
    i32 x, y, w, h;
    u32 color;
} draw_rectangle;

typedef struct draw_rectangle_textured {
    i32 x, y, w, h;   /* destination rectangle on screen */
    u8* pixels;       /* pointer to RGB pixel buffer (3 bytes per pixel) */
    u32 tex_w;        /* width of the source texture in pixels */
    u32 tex_h;        /* height of the source texture in pixels */
} draw_rectangle_textured;

typedef struct draw_command {
    draw_command_type type;
    union {
        draw_clear_background clear_bg;
        draw_rectangle rect;
        draw_rectangle_textured rect_textured;
    } data;
} draw_command;

/* Render the snake state into a sequence of draw_command stored in the
 * provided stack allocator. Returns a pointer to the first command and writes
 * the number of commands into command_count.
 */
draw_command* snake_render(snake* s, snake_asset* asset, u32 screen_width, u32 screen_height, u32* command_count, stack_alloc* alloc);

#endif /* SNAKE_RENDER_H */

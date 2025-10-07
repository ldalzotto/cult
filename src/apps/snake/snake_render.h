
#ifndef SNAKE_RENDER_H
#define SNAKE_RENDER_H

#include "stack_alloc.h"

typedef struct snake snake;

/* Draw command API for snake rendering. */

typedef enum {
    DRAW_COMMAND_CLEAR_BACKGROUND,
    DRAW_COMMAND_DRAW_RECTANGLE
} draw_command_type;

typedef struct draw_clear_background {
    u32 color; // RGBA
} draw_clear_background;

typedef struct draw_rectangle {
    i32 x, y, w, h;
    u32 color;
} draw_rectangle;

typedef struct draw_command {
    draw_command_type type;
    union {
        draw_clear_background clear_bg;
        draw_rectangle rect;
    } data;
} draw_command;

/* Render the snake state into a sequence of draw_command stored in the
 * provided stack allocator. Returns a pointer to the first command and writes
 * the number of commands into command_count.
 */
draw_command* snake_render(snake* s, u32 screen_width, u32 screen_height, u32* command_count, stack_alloc* alloc);

#endif /* SNAKE_RENDER_H */


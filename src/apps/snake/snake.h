#ifndef SNAKE_H
#define SNAKE_H

#include "stack_alloc.h"

typedef struct snake snake;
snake* snake_init(stack_alloc* alloc);
void snake_deinit(snake* s, stack_alloc* alloc);

// The end pointer of the snake module.
void* snake_end(snake* s);

typedef struct {
    u64 delta_time_between_movement;
} snake_config;
void snake_set_config(snake* s, snake_config config);

typedef struct {
    /* Input direction. */
    u8 left, right, up, down;
} snake_input;

typedef enum {
    SNAKE_UPDATE_CONTINUE,
    SNAKE_UPDATE_STOP
} snake_update_result;

snake_update_result snake_update(snake* s, snake_input input, u64 frame_us, stack_alloc* alloc);

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

draw_command* snake_render(snake* s, u32 screen_width, u32 screen_height, u32* command_count, stack_alloc* alloc);

#endif /*SNAKE_H*/

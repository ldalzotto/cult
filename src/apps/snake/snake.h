#ifndef SNAKE_H
#define SNAKE_H

#include "stack_alloc.h"
#include "snake_grid.h"

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

/* Accessors for opaque snake state (used by renderer and other modules). */
i32 snake_get_grid_width(snake* s);
i32 snake_get_grid_height(snake* s);
void snake_get_player_cells(snake* s, position** begin, position** end);
position snake_get_reward(snake* s);

#endif /*SNAKE_H*/

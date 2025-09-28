#ifndef SNAKE_H
#define SNAKE_H

#include "stack_alloc.h"

typedef struct snake snake;
snake* snake_init(stack_alloc* alloc);
void snake_deinit(snake* s, stack_alloc* alloc);
void snake_update(snake* s, u64 frame_ms, stack_alloc* alloc);

u32 snake_get_grid_width(const snake* s);
u32 snake_get_grid_height(const snake* s);
u32 snake_get_head_x(const snake* s);
u32 snake_get_head_y(const snake* s);

#endif /*SNAKE_H*/

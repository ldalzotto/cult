#ifndef SNAKE_GRID_H
#define SNAKE_GRID_H

#include "primitive.h"

typedef struct {
    i32 x;
    i32 y;
} position;

u8 snake_grid_inside(position position, i32 grid_width, i32 grid_height);
u8 snake_grid_equals(position left, position right);

#endif /*SNAKE_GRID_H*/

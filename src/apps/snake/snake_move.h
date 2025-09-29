#ifndef SNAKE_MOVE_H
#define SNAKE_MOVE_H

#include "snake_grid.h"
#include "snake_direction.h"

u8 snake_move_head(const position* begin, const position* end, snake_direction direction,
    u32 grid_width, u32 grid_height, position* out);

#endif /*SNAKE_MOVE_H*/

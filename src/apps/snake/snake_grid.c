#include "snake_grid.h"

position snake_grid_clamp(position position, i32 grid_width, i32 grid_height) {
    if (position.x >= grid_width) position.x = grid_width - 1;
    if (position.x < 0) position.x = 0;
    if (position.y >= grid_height) position.y = grid_height - 1;
    if (position.y < 0) position.y = 0;
    return position;
}

u8 snake_grid_equals(position left, position right) {
    return left.x == right.x && left.y == right.y;
}

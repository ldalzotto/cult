#include "snake_grid.h"

u8 snake_grid_inside(position position, i32 grid_width, i32 grid_height) {
    if (position.x >= grid_width) return 0;
    if (position.x < 0) return 0;
    if (position.y >= grid_height) return 0;
    if (position.y < 0) return 0;
    return 1;
}

u8 snake_grid_equals(position left, position right) {
    return left.x == right.x && left.y == right.y;
}

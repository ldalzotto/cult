#include "snake_move.h"

u8 snake_move_head(const position* begin, const position* end, snake_direction direction,
            u32 grid_width, u32 grid_height, position* out) {
    // Compute new head position based on current direction
    position head_pos = *begin;
    switch (direction) {
        case SNAKE_DIR_LEFT: head_pos.x -= 1; break;
        case SNAKE_DIR_RIGHT: head_pos.x += 1; break;
        case SNAKE_DIR_UP: head_pos.y -= 1; break;
        case SNAKE_DIR_DOWN: head_pos.y += 1; break;
    }

    if (!snake_grid_inside(head_pos, grid_width, grid_height)) {
        return 0;
    }

    u8 has_moved = 0;
    if (!snake_grid_equals(head_pos, *begin)) {
        has_moved = 1;
    }

    if (has_moved) {
        // Exclude the tail because the player hasn't moved yet
        for (const position* cell = begin; cell < end - 1; ++cell) {
            if (snake_grid_equals(head_pos, *cell)) {
                // Self-intersection detected; abort update
                return 0;
            }
        }
    }
    *out = head_pos;
    return 1;
}




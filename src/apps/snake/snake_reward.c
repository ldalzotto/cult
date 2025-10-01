#include "snake_reward.h"

position generate_next_reward_position(position current_reward, position* player_begin, position* player_end, u32 grid_width, u32 grid_height) {
    position candidate = current_reward;
    for (position* p = player_begin; p<player_end;++p) {
        i8 x_sign = p->x % 2 == 0 ? 1 : -1;
        i8 y_sign = p->y % 2 == 0 ? 1 : -1;
        candidate = (position){
            candidate.x + p->x * x_sign,
            candidate.y + p->y * y_sign,
        };
    }
    return (position) {
        candidate.x % grid_width,
        candidate.y % grid_height,
    };
}

position ensure_reward_position_not_on_player(position reward, position* begin, position* end,
                                        i32 grid_height, i32 grid_width) {
    i32 cand_x = reward.x;
    i32 cand_y = reward.y;

    u8 coll = 0;
    for (position* cell = begin; cell < end; ++cell) {
        if (cell->x == cand_x && cell->y == cand_y) { coll = 1; break; }
    }
    if (coll) {
        // Find the nearest cell position from cand_x, cand_y that is not intersecting with the player
        i32 best_x = cand_x;
        i32 best_y = cand_y;
        const i32 max_dist = grid_height * grid_width;
        i32 best_dist = max_dist;

        for (i32 yy = 0; yy < grid_height; ++yy) {
            for (i32 xx = 0; xx < grid_width; ++xx) {
                // skip positions that collide with player
                u8 on_player = 0;
                for (position* cell = begin; cell < end; ++cell) {
                    if (cell->x == xx && cell->y == yy) {
                        on_player = 1;
                        break;
                    }
                }
                if (on_player) continue;

                i32 dx = xx - cand_x;
                if (dx < 0) dx = -dx;
                i32 dy = yy - cand_y;
                if (dy < 0) dy = -dy;
                i32 dist = dx + dy;

                if (dist < best_dist) {
                    best_dist = dist;
                    best_x = xx;
                    best_y = yy;
                }
            }
        }

        if (best_dist != max_dist) {
            return (position) {best_x, best_y};
        }
    }
    return reward;
}

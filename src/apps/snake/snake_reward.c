#include "snake_reward.h"

static u8 position_on_player(position p, position* begin, position* end) {
    u8 coll = 0;
    for (position* cell = begin; cell < end; ++cell) {
        if (snake_grid_equals(*cell, p)) { coll = 1; break; }
    }
    return coll;
}

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
    position cand = reward;

    if (position_on_player(cand, begin, end)) {
        // Find the nearest cell position from cand_x, cand_y that is not intersecting with the player
        position best = cand;
        const i32 max_dist = grid_height * grid_width;
        i32 best_dist = max_dist;

        for (i32 yy = 0; yy < grid_height; ++yy) {
            for (i32 xx = 0; xx < grid_width; ++xx) {
                // skip positions that collide with player
                if (position_on_player((position){xx, yy}, begin, end)) {
                    continue;
                }

                i32 dx = xx - cand.x;
                if (dx < 0) dx = -dx;
                i32 dy = yy - cand.y;
                if (dy < 0) dy = -dy;
                i32 dist = dx + dy;

                if (dist < best_dist) {
                    best_dist = dist;
                    best.x = xx;
                    best.y = yy;
                }
            }
        }

        if (best_dist != max_dist) {
            return best;
        }
    }
    return reward;
}

#ifndef SNAKE_REWARD_H
#define SNAKE_REWARD_H

#include "primitive.h"
#include "snake_grid.h"

position generate_next_reward_position(position current_reward, position* player_begin, position* player_end, u32 grid_width, u32 grid_height);
position ensure_reward_position_not_on_player(position reward, position* begin, position* end, i32 grid_height, i32 grid_width);

#endif

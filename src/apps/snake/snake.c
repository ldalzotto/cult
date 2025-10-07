#include "snake.h"
#include "primitive.h"
#include "snake_grid.h"
#include "assert.h"
#include "snake_move.h"
#include "snake_reward.h"

struct snake {
    position reward;
    
    i32 grid_width;
    i32 grid_height;

    // Time accumulator to perform updates at a fixed interval
    u64 time_accum_us;
    u64 delta_time_between_movement;

    struct {
        position* begin;
        position* end;
    } player_cells;
    snake_direction player_direction;
    
    void* end;
};

snake* snake_init(stack_alloc* alloc) {
    snake* s = sa_alloc(alloc, sizeof(*s));
    s->grid_width = 20;
    s->grid_height = 20;

    s->player_cells.begin = sa_alloc(alloc, sizeof(*s->player_cells.begin));
    s->player_cells.end = byteoffset(s->player_cells.begin, sizeof(*s->player_cells.begin));

    position* start_position = s->player_cells.begin;
    start_position->x = s->grid_width / 2;
    start_position->y = s->grid_height / 2;

    s->reward.x = s->grid_width / 3;
    s->reward.y = s->grid_height / 3;
    s->end = alloc->cursor;
    s->player_direction = SNAKE_DIR_RIGHT;
    s->time_accum_us = 0;
    s->delta_time_between_movement = 1000000 / 8;
    return s;
}

void snake_deinit(snake* s, stack_alloc* alloc) {
    sa_free(alloc, s->player_cells.begin);
    sa_free(alloc, s);
}

void* snake_end(snake* s) {
    return s->end;
}

void snake_set_config(snake* s, snake_config config) {
    s->delta_time_between_movement = config.delta_time_between_movement;
}

snake_update_result snake_update(snake* s, snake_input input, u64 frame_us, stack_alloc* alloc) {
    // Determine new direction from input
    snake_direction direction_new = s->player_direction;
    if (input.left) direction_new = SNAKE_DIR_LEFT;
    else if (input.right) direction_new = SNAKE_DIR_RIGHT;
    else if (input.up) direction_new = SNAKE_DIR_UP;
    else if (input.down) direction_new = SNAKE_DIR_DOWN;

    // If the new direction is directly opposite to the old direction, ignore the
    // change to prevent the snake from reversing into itself.
    snake_direction direction_old = s->player_direction;
    if ((direction_old == SNAKE_DIR_LEFT  && direction_new == SNAKE_DIR_RIGHT) ||
        (direction_old == SNAKE_DIR_RIGHT && direction_new == SNAKE_DIR_LEFT)  ||
        (direction_old == SNAKE_DIR_UP    && direction_new == SNAKE_DIR_DOWN) ||
        (direction_old == SNAKE_DIR_DOWN  && direction_new == SNAKE_DIR_UP)) {
        direction_new = direction_old;
    }

    s->player_direction = direction_new;

    // Accumulate time and perform update only when at least 1 second has elapsed
    
    s->time_accum_us += frame_us;
    if (s->time_accum_us < s->delta_time_between_movement) {
        return SNAKE_UPDATE_CONTINUE;
    }
    s->time_accum_us -= s->delta_time_between_movement;

    position head_pos;
    if (!snake_move_head(s->player_cells.begin, s->player_cells.end, s->player_direction, 
                            s->grid_width, s->grid_height, &head_pos)) {
        // No valid movement found -> stop game
        return SNAKE_UPDATE_STOP;
    }

    /* Check for reward collection and update reward position deterministically within bounds. */
    u8 should_extend = 0;
    if (snake_grid_equals(head_pos, s->reward)) {
        should_extend = 1;
    }

    // Implement basic multi-cell support
    if (should_extend) {
        debug_assert(s->player_cells.end == alloc->cursor);
        void* next_player_cells_end = byteoffset(s->player_cells.end, sizeof(*s->player_cells.begin));
        {
            sa_insert(alloc, s->player_cells.end, sizeof(*s->player_cells.begin));
        }

        s->player_cells.end = next_player_cells_end;
        s->end = byteoffset(s->end, sizeof(*s->player_cells.begin));

        // Shift existing cells to follow the head within the current length
        for (position* cell = s->player_cells.end - 1; cell > s->player_cells.begin; --cell) {
            *cell = *(cell - 1);
        }

        *s->player_cells.begin = head_pos;
    } else {
        // Shift existing cells to follow the head within the current length
        for (position* cell = s->player_cells.end - 1; cell > s->player_cells.begin; --cell) {
            *cell = *(cell - 1);
        }
        *s->player_cells.begin = head_pos;
    }

    // TODO: add a test about that
    // Generate new reward position
    if (should_extend) {
        position cand = generate_next_reward_position(s->reward, s->player_cells.begin, s->player_cells.end,
            s->grid_width, s->grid_height);
        s->reward =  ensure_reward_position_not_on_player(cand, s->player_cells.begin, s->player_cells.end,
            s->grid_width, s->grid_height);
    }

    return SNAKE_UPDATE_CONTINUE;
}

/* Accessor implementations */

i32 snake_get_grid_width(snake* s) {
    return s->grid_width;
}

i32 snake_get_grid_height(snake* s) {
    return s->grid_height;
}

void snake_get_player_cells(snake* s, position** begin, position** end) {
    if (begin) *begin = s->player_cells.begin;
    if (end)   *end = s->player_cells.end;
}

position snake_get_reward(snake* s) {
    return s->reward;
}

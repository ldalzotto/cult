#include "snake.h"
#include "primitive.h"
#include "snake_grid.h"
#include "assert.h"

typedef enum {
    SNAKE_DIR_LEFT,
    SNAKE_DIR_RIGHT,
    SNAKE_DIR_UP,
    SNAKE_DIR_DOWN
} snake_direction;

struct snake {
    position reward;
    i32 score;
    
    i32 grid_width;
    i32 grid_height;

    struct {
        position* begin;
        position* end;
    } player_cells;
    snake_direction player_direction;
    void* end;

    // Time accumulator to perform updates at a fixed interval (1 second)
    u64 time_accum_us;
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
    s->score = 0;
    s->end = alloc->cursor;
    s->player_direction = SNAKE_DIR_RIGHT;
    s->time_accum_us = 0;
    return s;
}

void snake_deinit(snake* s, stack_alloc* alloc) {
    sa_free(alloc, s->player_cells.begin);
    sa_free(alloc, s);
}

void* snake_end(snake* s) {
    return s->end;
}

void snake_update(snake* s, snake_input input, u64 frame_us, stack_alloc* alloc) {
    // Determine new direction from input
    snake_direction next_dir = s->player_direction;
    if (input.left) next_dir = SNAKE_DIR_LEFT;
    else if (input.right) next_dir = SNAKE_DIR_RIGHT;
    else if (input.up) next_dir = SNAKE_DIR_UP;
    else if (input.down) next_dir = SNAKE_DIR_DOWN;

    s->player_direction = next_dir;

    // Accumulate time and perform update only when at least 1 second has elapsed
    const u64 delta_between_movement = 1000000 / 8;
    s->time_accum_us += frame_us;
    if (s->time_accum_us < delta_between_movement) {
        return;
    }
    s->time_accum_us -= delta_between_movement;

    // Compute new head position based on current direction
    position head_pos = *s->player_cells.begin;
    switch (s->player_direction) {
        case SNAKE_DIR_LEFT: head_pos.x -= 1; break;
        case SNAKE_DIR_RIGHT: head_pos.x += 1; break;
        case SNAKE_DIR_UP: head_pos.y -= 1; break;
        case SNAKE_DIR_DOWN: head_pos.y += 1; break;
    }

    if (!snake_grid_inside(head_pos, s->grid_width, s->grid_height)) {
        return;
    }

    u8 has_moved = 0;
    if (!snake_grid_equals(head_pos, *s->player_cells.begin)) {
        has_moved = 1;
    }

    if (has_moved) {
        // Exclude the tail because the player hasn't moved yet
        for (position* cell = s->player_cells.begin; cell < s->player_cells.end - 1; ++cell) {
            if (snake_grid_equals(head_pos, *cell)) {
                // Self-intersection detected; abort update
                return;
            }
        }
    }

    /* Check for reward collection and update reward position deterministically within bounds. */
    u8 should_extend = 0;
    if (snake_grid_equals(head_pos, s->reward)) {
        s->score++;
        s->reward.x = (s->reward.x + 3) % s->grid_width;
        s->reward.y = (s->reward.y + 4) % s->grid_height;
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
    } else if (has_moved) {
        // Shift existing cells to follow the head within the current length
        for (position* cell = s->player_cells.end - 1; cell > s->player_cells.begin; --cell) {
            *cell = *(cell - 1);
        }
        *s->player_cells.begin = head_pos;
    }
}

draw_command* snake_render(snake* s, u32 screen_width, u32 screen_height, u32* command_count, stack_alloc* alloc) {
    draw_command* cmds = alloc->cursor;
    
    // Clear background
    {
        draw_command* c = sa_alloc(alloc, sizeof(*c));
        c->type = DRAW_COMMAND_CLEAR_BACKGROUND;
        c->data.clear_bg.color = 0x00000000; // Black
    }

    // Draw snake head rectangle
    i32 cell_size_x = screen_width / s->grid_width;
    i32 cell_size_y = screen_height / s->grid_height;
    for (position* cell = s->player_cells.begin; cell < s->player_cells.end; ++cell) {
        draw_command* c = sa_alloc(alloc, sizeof(*c));
        c->type = DRAW_COMMAND_DRAW_RECTANGLE;
        c->data.rect.x = (i32)cell->x * cell_size_x;
        c->data.rect.y = (i32)cell->y * cell_size_y;
        c->data.rect.w = cell_size_x;
        c->data.rect.h = cell_size_y;
        c->data.rect.color = 0x00FFFFFF; // White
    }
    
    // Render the reward rectangle
    {
        draw_command* c = sa_alloc(alloc, sizeof(*c));
        c->type = DRAW_COMMAND_DRAW_RECTANGLE;
        c->data.rect.x = (i32)s->reward.x * cell_size_x;
        c->data.rect.y = (i32)s->reward.y * cell_size_y;
        c->data.rect.w = cell_size_x;
        c->data.rect.h = cell_size_y;
        c->data.rect.color = 0x0000FF00; // Green
    }

    *command_count = bytesize(cmds, alloc->cursor) / sizeof(draw_command);

    return cmds;
}

#include "snake.h"
#include "primitive.h"
#include "snake_grid.h"
#include "assert.h"

struct snake {
    position reward;
    i32 score;
    
    i32 grid_width;
    i32 grid_height;

    struct {
        position* begin;
        position* end;
    } player_cells;
    /*[USER] We should hold a current direction state. Which says left,right,up,down*/
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
    s->score = 0;
    s->end = alloc->cursor;
    return s;
}

void snake_deinit(snake* s, stack_alloc* alloc) {
    sa_free(alloc, s->player_cells.begin);
    sa_free(alloc, s);
}

void* snake_end(snake* s) {
    return s->end;
}

void snake_update(snake* s, snake_input input, u64 frame_ms, stack_alloc* alloc) {
    unused(frame_ms);

    /* Move the head of the snake based on input. The current implementation uses a single head
    stored at player_cells.begin. We update that head position and clamp to grid. */
    // Determine new head position
    position head_pos = *s->player_cells.begin;
    /*[USER] Update the direction state instead*/
    u8 has_moved = 0;
    if (input.left) {
        has_moved = 1;
        head_pos.x--;
    } else if (input.right) {
        has_moved = 1;
        head_pos.x++;
    } else if (input.up) {
        has_moved = 1;
        head_pos.y--;
    } else if (input.down) {
        has_moved = 1;
        head_pos.y++;
    }


    /*[USER] Use the direction state to compute the head_pos*/

    if (!snake_grid_inside(head_pos, s->grid_width, s->grid_height)) {
        return;
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

#include "snake.h"
#include "primitive.h"
#include "snake_grid.h"

struct snake {
    position reward;
    i32 score;
    
    i32 grid_width;
    i32 grid_height;

    struct {
        position* begin;
        position* end;
    } player_cells;
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
    unused(alloc);

    /* Move the head of the snake based on input. The current implementation uses a single head
    stored at player_cells.begin. We update that head position and clamp to grid. */
    position head_pos = *s->player_cells.begin;
    if (input.left) {
        head_pos.x--;
    } else if (input.right) {
        head_pos.x++;
    } else if (input.up) {
        head_pos.y--;
    } else if (input.down) {
        head_pos.y++;
    }

    head_pos = snake_grid_clamp(head_pos, s->grid_width, s->grid_height);
    *s->player_cells.begin = head_pos;

    /* Check for reward collection and update reward position deterministically within bounds. */
    u8 should_extend = 0;
    if (snake_grid_equals(head_pos, s->reward)) {
        s->score++;
        s->reward.x = (s->reward.x + 3) % s->grid_width;
        s->reward.y = (s->reward.y + 4) % s->grid_height;
        should_extend = 1;
    }

    // Growth not implemented: keeping single-segment snake. If growth is required, more memory and
    // data structure updates would be needed.
    if (should_extend) {
        // No extension logic implemented yet
    }

}

draw_command* snake_render(snake* s, u32 screen_width, u32 screen_height, u32* command_count, stack_alloc* alloc) {
    draw_command* cmds = sa_alloc(alloc, 3 * sizeof(draw_command));
    *command_count = 3;

    // Clear background
    cmds[0].type = DRAW_COMMAND_CLEAR_BACKGROUND;
    cmds[0].data.clear_bg.color = 0x00000000; // Black

    // Draw snake head rectangle
    i32 cell_size_x = screen_width / s->grid_width;
    i32 cell_size_y = screen_height / s->grid_height;
    cmds[1].type = DRAW_COMMAND_DRAW_RECTANGLE;
    cmds[1].data.rect.x = (i32)s->player_cells.begin->x * cell_size_x;
    cmds[1].data.rect.y = (i32)s->player_cells.begin->y * cell_size_y;
    cmds[1].data.rect.w = cell_size_x;
    cmds[1].data.rect.h = cell_size_y;
    cmds[1].data.rect.color = 0x00FFFFFF; // White

        // Render the reward rectangle
    cmds[2].type = DRAW_COMMAND_DRAW_RECTANGLE;
    cmds[2].data.rect.x = (i32)s->reward.x * cell_size_x;
    cmds[2].data.rect.y = (i32)s->reward.y * cell_size_y;
    cmds[2].data.rect.w = cell_size_x;
    cmds[2].data.rect.h = cell_size_y;
    cmds[2].data.rect.color = 0x0000FF00; // Green

    return cmds;
}

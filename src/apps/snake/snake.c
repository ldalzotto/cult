#include "snake.h"
#include "primitive.h"
#include "snake_grid.h"

struct snake {
    position head;
    position reward;
    i32 grid_width;
    i32 grid_height;
    i32 score;
};

snake* snake_init(stack_alloc* alloc) {
    snake* s = sa_alloc(alloc, sizeof(*s));
    s->grid_width = 20;
    s->grid_height = 20;
    s->head.x = s->grid_width / 2;
    s->head.y = s->grid_height / 2;
    s->reward.x = s->grid_width / 3;
    s->reward.y = s->grid_height / 3;
    s->score = 0;
    return s;
}

void snake_deinit(snake* s, stack_alloc* alloc) {
    sa_free(alloc, s);
}

void snake_update(snake* s, snake_input input, u64 frame_ms, stack_alloc* alloc) {
    unused(frame_ms);
    unused(alloc);
    if (input.left) {
        s->head.x--;
    } else if (input.right) {
        s->head.x++;
    } else if (input.up) {
        s->head.y--;
    } else if (input.down) {
        s->head.y++;
    }

    /* Ensure the position stays inside the grid */

    s->head = snake_grid_clamp(s->head, s->grid_width, s->grid_height);

    if (snake_grid_equals(s->head, s->reward)) {
        s->score++;
        // relocate reward to a new position within the grid bounds in a deterministic way
        s->reward.x = (s->reward.x + 3) % s->grid_width;
        s->reward.y = (s->reward.y + 4) % s->grid_height;
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
    cmds[1].data.rect.x = (i32)s->head.x * cell_size_x;
    cmds[1].data.rect.y = (i32)s->head.y * cell_size_y;
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

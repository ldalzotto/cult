#include "snake.h"
#include "primitive.h"

struct snake {
    i32 head_x;
    i32 head_y;
    i32 grid_width;
    i32 grid_height;
};

snake* snake_init(stack_alloc* alloc) {
    snake* s = sa_alloc(alloc, sizeof(*s));
    s->grid_width = 20;
    s->grid_height = 20;
    s->head_x = s->grid_width / 2;
    s->head_y = s->grid_height / 2;
    return s;
}

void snake_deinit(snake* s, stack_alloc* alloc) {
    sa_free(alloc, s);
}

void snake_update(snake* s, snake_input input, u64 frame_ms, stack_alloc* alloc) {
    unused(frame_ms);
    unused(alloc);
    if (input.left) {
        s->head_x--;
    } else if (input.right) {
        s->head_x++;
    } else if (input.up) {
        s->head_y--;
    } else if (input.down) {
        s->head_y++;
    }

    /* Ensure the position stays inside the grid */
    if (s->head_x >= s->grid_width) s->head_x = s->grid_width - 1;
    if (s->head_x < 0) s->head_x = 0;
    if (s->head_y >= s->grid_height) s->head_y = s->grid_height - 1;
    if (s->head_y < 0) s->head_y = 0;
}

draw_command* snake_render(snake* s, u32 screen_width, u32 screen_height, u32* command_count, stack_alloc* alloc) {
    draw_command* cmds = sa_alloc(alloc, 2 * sizeof(draw_command));
    *command_count = 2;

    // Clear background
    cmds[0].type = DRAW_COMMAND_CLEAR_BACKGROUND;
    cmds[0].data.clear_bg.color = 0x00000000; // Black

    // Draw snake head rectangle
    i32 cell_size_x = screen_width / s->grid_width;
    i32 cell_size_y = screen_height / s->grid_height;
    cmds[1].type = DRAW_COMMAND_DRAW_RECTANGLE;
    cmds[1].data.rect.x = (i32)s->head_x * cell_size_x;
    cmds[1].data.rect.y = (i32)s->head_y * cell_size_y;
    cmds[1].data.rect.w = cell_size_x;
    cmds[1].data.rect.h = cell_size_y;
    cmds[1].data.rect.color = 0x00FFFFFF; // White

    return cmds;
}

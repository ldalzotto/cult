#include "snake.h"
#include "primitive.h"

struct snake {
    u32 head_x;
    u32 head_y;
    u32 grid_width;
    u32 grid_height;
    u8* grid;
};


snake* snake_init(stack_alloc* alloc) {
    snake* s = sa_alloc(alloc, sizeof(*s));
    s->grid_width = 10;
    s->grid_height = 10;
    s->grid = sa_alloc(alloc, s->grid_width * s->grid_height * sizeof(u8));
    s->head_x = s->grid_width / 2;
    s->head_y = s->grid_height / 2;
    return s;
}

void snake_deinit(snake* s, stack_alloc* alloc) {
    sa_free(alloc, s->grid);
    sa_free(alloc, s);
}

void snake_update(snake* s, u64 frame_ms, stack_alloc* alloc) {
    unused(s);
    unused(frame_ms);
    unused(alloc);
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

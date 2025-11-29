#include "snake_render.h"
#include "primitive.h"
#include "snake_grid.h"
#include "snake.h"

static void snake_render_enqueue_cell(stack_alloc* alloc, position cell, i32 cell_size_x, i32 cell_size_y, image texture) {
    draw_command* c = sa_alloc(alloc, sizeof(*c));
    c->type = DRAW_COMMAND_DRAW_RECTANGLE_TEXTURED;
    c->data.rect_textured.x = (i32)cell.x * cell_size_x;
    c->data.rect_textured.y = (i32)cell.y * cell_size_y;
    c->data.rect_textured.w = cell_size_x;
    c->data.rect_textured.h = cell_size_y;
    c->data.rect_textured.pixels = (u8*)texture.data;
    c->data.rect_textured.tex_w = texture.width;
    c->data.rect_textured.tex_h = texture.height;
}

draw_command* snake_render(snake* s, snake_asset* asset, u32 screen_width, u32 screen_height, u32* command_count, stack_alloc* alloc) {
    draw_command* cmds = alloc->cursor;
    
    // Clear background
    {
        draw_command* c = sa_alloc(alloc, sizeof(*c));
        c->type = DRAW_COMMAND_CLEAR_BACKGROUND;
        c->data.clear_bg.color = 0x00000000; // Black
    }

    // Draw snake rectangles
    i32 grid_w = snake_get_grid_width(s);
    i32 grid_h = snake_get_grid_height(s);
    i32 cell_size_x = screen_width / grid_w;
    i32 cell_size_y = screen_height / grid_h;

    position* begin = 0;
    position* end = 0;
    snake_get_player_cells(s, &begin, &end);

    image head_texture = snake_asset_head(asset);
    image body_texture = snake_asset_body(asset);
    image tail_texture = snake_asset_tail(asset);
    if (begin && begin < end) {
        snake_render_enqueue_cell(alloc, *begin, cell_size_x, cell_size_y, head_texture);
        if (begin + 1 < end) {
            position* tail = end - 1;
            for (position* cell = begin + 1; cell < tail; ++cell) {
                snake_render_enqueue_cell(alloc, *cell, cell_size_x, cell_size_y, body_texture);
            }
            snake_render_enqueue_cell(alloc, *tail, cell_size_x, cell_size_y, tail_texture);
        }
    }
    
    // Render the reward rectangle
    {
        position reward = snake_get_reward(s);
        draw_command* c = sa_alloc(alloc, sizeof(*c));
        c->type = DRAW_COMMAND_DRAW_RECTANGLE_TEXTURED;
        c->data.rect_textured.x = (i32)reward.x * cell_size_x;
        c->data.rect_textured.y = (i32)reward.y * cell_size_y;
        c->data.rect_textured.w = cell_size_x;
        c->data.rect_textured.h = cell_size_y;
        image apple = snake_asset_apple(asset);
        c->data.rect_textured.pixels = apple.data;
        c->data.rect_textured.tex_w = apple.width;
        c->data.rect_textured.tex_h = apple.height;
    }

    *command_count = bytesize(cmds, alloc->cursor) / sizeof(draw_command);

    return cmds;
}

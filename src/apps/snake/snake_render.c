#include "snake_render.h"
#include "primitive.h"
#include "snake_grid.h"
#include "snake.h"

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
    // TODO: is the cell is the first, draw head, else draw tail
    for (position* cell = begin; cell < end; ++cell) {
        draw_command* c = sa_alloc(alloc, sizeof(*c));
        c->type = DRAW_COMMAND_DRAW_RECTANGLE_TEXTURED;
        c->data.rect_textured.x = (i32)cell->x * cell_size_x;
        c->data.rect_textured.y = (i32)cell->y * cell_size_y;
        c->data.rect_textured.w = cell_size_x;
        c->data.rect_textured.h = cell_size_y;
        image body = snake_asset_body(asset);
        c->data.rect_textured.pixels = body.data;
        c->data.rect_textured.tex_w = body.width;
        c->data.rect_textured.tex_h = body.height;
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

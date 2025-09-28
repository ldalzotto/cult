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

u32 snake_get_grid_width(const snake* s) {
    return s->grid_width;
}

u32 snake_get_grid_height(const snake* s) {
    return s->grid_height;
}

u32 snake_get_head_x(const snake* s) {
    return s->head_x;
}

u32 snake_get_head_y(const snake* s) {
    return s->head_y;
}

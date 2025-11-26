#ifndef SNAKE_ASSET_H
#define SNAKE_ASSET_H

#include "stack_alloc.h"

typedef struct snake_asset snake_asset;
snake_asset* snake_asset_init(stack_alloc* alloc);
void snake_asset_deinit(snake_asset* asset, stack_alloc* alloc);

void snake_asset_default_image(snake_asset* asset, void** out_buffer, i32* out_width, i32* out_height);

#endif

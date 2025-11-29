#ifndef SNAKE_ASSET_H
#define SNAKE_ASSET_H

#include "stack_alloc.h"

typedef struct snake_asset snake_asset;
snake_asset* snake_asset_init(stack_alloc* alloc);
void snake_asset_deinit(snake_asset* asset, stack_alloc* alloc);

typedef struct {
    void* data;
    u32 width;
    u32 height;
} image;

image snake_asset_apple(snake_asset* asset);
image snake_asset_body(snake_asset* asset);

#endif

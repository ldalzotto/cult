
#include "snake_asset.h"

struct snake_asset {
    void* default_buffer;
    i32 default_width;
    i32 default_height;
};

snake_asset* snake_asset_init(stack_alloc* alloc) {
    snake_asset* asset = sa_alloc(alloc, sizeof(*asset));
    asset->default_width = 8;
    asset->default_height = 8;
    asset->default_buffer = sa_alloc(alloc, asset->default_width * asset->default_height * 3);
    // Create a gradient
    {
        unsigned char* buf = (unsigned char*)asset->default_buffer;
        i32 w = asset->default_width;
        i32 h = asset->default_height;
        for (i32 y = 0; y < h; ++y) {
            for (i32 x = 0; x < w; ++x) {
                int idx = (y * w + x) * 3;
                unsigned char r = (unsigned char)((x * 255) / (w - 1));
                unsigned char g = (unsigned char)((y * 255) / (h - 1));
                unsigned char b = (unsigned char)(((x + y) * 255) / (w + h - 2));
                buf[idx + 0] = r;
                buf[idx + 1] = g;
                buf[idx + 2] = b;
            }
        }
    }
    return asset;
}

void snake_asset_deinit(snake_asset* asset, stack_alloc* alloc) {
    sa_free(alloc, asset);
}

void snake_asset_default_image(snake_asset* asset, void** out_buffer, i32* out_width, i32* out_height) {
    *out_buffer = asset->default_buffer;
    *out_width = asset->default_width;
    *out_height = asset->default_height;
}

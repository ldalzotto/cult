
#include "snake_asset.h"

struct snake_asset {
    image default_image;
};

snake_asset* snake_asset_init(stack_alloc* alloc) {
    snake_asset* asset = sa_alloc(alloc, sizeof(*asset));
    asset->default_image.width = 8;
    asset->default_image.height = 8;
    asset->default_image.data = sa_alloc(alloc, asset->default_image.width * asset->default_image.height * 3);
    // Create a gradient
    {
        unsigned char* buf = asset->default_image.data;
        u32 w = asset->default_image.width;
        u32 h = asset->default_image.height;
        for (u32 y = 0; y < h; ++y) {
            for (u32 x = 0; x < w; ++x) {
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

image snake_asset_default_image(snake_asset* asset) {
    return asset->default_image;
}

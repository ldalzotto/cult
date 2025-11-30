
#include "snake_asset.h"

struct snake_asset {
    image apple;
    image body;
    image head;
    image tail;
};

typedef struct rgb888 {u8 r; u8 g; u8 b; } rgb888;

static image allocate_apple(void) {
    const rgb888 black = {0x00, 0x00, 0x00};
    const rgb888 green_light = {0x00, 0xE4, 0x36};
    const rgb888 green_dark = {0x00, 0x07, 0x51};
    const rgb888 red_light = {0xFF, 0x00, 0x40};
    const rgb888 red_dark = {0x7E, 0x25, 0x53};
    const rgb888 white = {0xFF, 0xF1, 0xE8};
    const rgb888 pink_light = {0xFF, 0x77, 0xA8};

    #define w 8
    #define h 8
    static const rgb888 APPLE[w*h] = {
        // row 0
        black,     black, black, green_light, black, black, black, black,
        // row 1
        black,     black, black, green_light, green_dark, black, black, black,
        // row 2
        black,     red_light, red_light, red_dark, green_light, red_light, red_dark, black, 
        // row 3
        red_light, white, pink_light, red_light,red_light,red_light,red_light, red_dark,
        // row 4
        red_light, pink_light, red_light, red_light,red_light,red_light,red_light, red_dark,
        // row 5
        red_light, red_light, red_light, red_light,red_light,red_light,red_light, red_dark,
        // row 6
        black,     red_light, red_light, red_light,red_light, red_light, red_dark, black,
        // row 7
        black,     black, red_light, red_light,red_light, red_dark, black, black,
    };
    image image;
    image.data = (void*)APPLE;
    image.width = w;
    image.height = h;

    #undef w
    #undef h

    return image;
}

static image allocate_snake_head(void) {
    const rgb888 black = {0x00, 0x00, 0x00};
    const rgb888 green_dark = {0x00, 0x94, 0x36};
    const rgb888 green = {0x22, 0xB1, 0x4C};
    const rgb888 green_light = {0x5E, 0xFC, 0x8B};
    const rgb888 white = {0xFF, 0xFF, 0xFF};
    const rgb888 pupil = {0x00, 0x00, 0x00};

    #define w 8
    #define h 8
    static const rgb888 HEAD[w*h] = {
        // row 0
        black, black, black, black, black, black, black, black,
        // row 1
        black, green_dark, green_dark, green_dark, green_dark, green_dark, green_dark, black,
        // row 2
        black, green_dark, green_light, green_light, green_light, green_light, green_dark, black,
        // row 3
        black, green, white, pupil, pupil, white, green, black,
        // row 4
        black, green, green, green, green, green, green, black,
        // row 5
        black, green, green, green_light, green_light, green, green, black,
        // row 6
        black, green, green, green, green, green, green, black,
        // row 7
        black, black, black, black, black, black, black, black,
    };
    image image;
    image.data = (void*)HEAD;
    image.height = h;
    image.width = w;

    #undef h
    #undef w

    return image;
}

static image allocate_snake_tail(void) {
    const rgb888 black = {0x00, 0x00, 0x00};
    const rgb888 tail_dark = {0x00, 0x74, 0x2A};
    const rgb888 tail_mid = {0x11, 0xA1, 0x4E};
    const rgb888 tail_light = {0x1B, 0xE0, 0x6E};

    #define w 8
    #define h 8
    static const rgb888 TAIL[w*h] = {
        // row 0
        black, black, black, black, black, black, black, black,
        // row 1
        black, tail_dark, tail_dark, tail_dark, tail_dark, tail_dark, tail_dark, black,
        // row 2
        black, tail_dark, tail_mid, tail_mid, tail_mid, tail_mid, tail_dark, black,
        // row 3
        black, tail_dark, tail_mid, tail_light, tail_light, tail_mid, tail_dark, black,
        // row 4
        black, tail_dark, tail_mid, tail_light, tail_light, tail_mid, tail_dark, black,
        // row 5
        black, tail_dark, tail_mid, tail_mid, tail_mid, tail_mid, tail_dark, black,
        // row 6
        black, tail_dark, tail_dark, tail_mid, tail_mid, tail_dark, tail_dark, black,
        // row 7
        black, black, black, black, black, black, black, black,
    };
    image image;
    image.data = (void*)TAIL;
    image.width = w;
    image.height = h;

    #undef w
    #undef h

    return image;
}

static image allocate_snake_body(void) {
    const rgb888 black = {0x00, 0x00, 0x00};
    const rgb888 white = {0xFF, 0xFF, 0xFF};
    const rgb888 green = {0x22, 0xB1, 0x4C};

    #define w 8
    #define h 8
    static const rgb888 BODY[w*h] = {
        // row 0
        black,     black, black, black, black, black, black, black,
        // row 1
        black,     white, green, green, green, green, white, black,
        // row 2
        black,     green, white, green, green, white, green, black, 
        // row 3
        black, green, green, white, white, green , green, black,
        // row 4
        black, green, green, white, white, green, green, black,
        // row 5
        black, green, white, green, green, white, green, black,
        // row 6
        black,     white, green, green, green, green, white, black,
        // row 7
        black,     black, black, black, black, black, black, black,
    };
    image image;
    image.data = (void*)BODY;
    image.width = w;
    image.height = h;

    #undef w
    #undef h

    return image;
}

snake_asset* snake_asset_init(stack_alloc* alloc) {
    snake_asset* asset = sa_alloc(alloc, sizeof(*asset));
    asset->apple = allocate_apple();
    asset->body = allocate_snake_body();
    asset->head = allocate_snake_head();
    asset->tail = allocate_snake_tail();
    return asset;
}

void snake_asset_deinit(snake_asset* asset, stack_alloc* alloc) {
    sa_free(alloc, asset);
}

image snake_asset_apple(snake_asset* asset) {
    return asset->apple;
}

image snake_asset_body(snake_asset* asset) {
    return asset->body;
}

image snake_asset_head(snake_asset* asset) {
    return asset->head;
}

image snake_asset_tail(snake_asset* asset) {
    return asset->tail;
}

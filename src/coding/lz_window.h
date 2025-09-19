#ifndef LZ_WINDOW_H
#define LZ_WINDOW_H

#include "primitive.h"

typedef struct {
    u8* search_begin;
    u8* lookahead_begin;
    u8* end;
} lz_window;

void lz_window_advance(lz_window* window, u8* to, uptr window_size_max);
u8 lz_window_end(lz_window window, uptr match_size_min);

#endif /* LZ_WINDOW_H */

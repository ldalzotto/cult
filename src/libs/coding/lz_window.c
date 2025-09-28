#include "lz_window.h"
#include "assert.h"

void lz_window_advance(lz_window* window, u8* to, lzss_window_size_t window_size_max) {
    debug_assert(to > window->lookahead_begin);
    debug_assert(to <= window->end);
    window->lookahead_begin = to;
    if (window->lookahead_begin > window->end) {
        window->lookahead_begin = window->end;
    }

    uptr window_size_current = bytesize(window->search_begin, window->lookahead_begin);
    if (window_size_current > window_size_max) {
        window->search_begin = byteoffset(window->lookahead_begin, -window_size_max);
    }
}

u8 lz_window_end(lz_window window, uptr match_size_min) {
    debug_assert(window.lookahead_begin <= window.end);
    uptr lookahead_size = bytesize(window.lookahead_begin, window.end);
    return lookahead_size < match_size_min;
}

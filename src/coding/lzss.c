#include "lzss.h"
#include "lz_window.h"
#include "lz_match_brute.h"
#include "print.h"

static void* compress(u8* begin, u8* end, stack_alloc* alloc, u8 debug) {
    const uptr match_size_min = 3;
    const uptr window_size_max = 1024;
    lz_window window = {
        .search_begin = begin,
        .lookahead_begin = begin,
        .end = end,
    };
    struct {lz_match* begin; lz_match* end;} matches;
    matches.begin = alloc->cursor;
    while (!lz_window_end(window, match_size_min)) {
        lz_match match = lz_match_brute(window);
        u8* lookahead_next;
        if (lz_match_has_value(match)) {
            *(lz_match*)sa_alloc(alloc, sizeof(match)) = match;
            lookahead_next = match.lookahead.end;
        } else {
            lookahead_next = byteoffset(window.lookahead_begin, 1);
        }
        lz_window_advance(&window, lookahead_next, window_size_max);
    }
    matches.end = alloc->cursor;
    unused(matches);

    if (debug) {
        for (lz_match* match = matches.begin; match<matches.end; ++match) {
            print_string(file_stdout(), (string){match->search.begin, match->search.end});
            print_string(file_stdout(), STRING("\n"));
        }
    }
    
    // TODO serialize

    return matches.begin;
}

void* lzss_compress(u8* begin, u8* end, stack_alloc* alloc, u8 debug) {
    return compress(begin, end, alloc, debug);
}


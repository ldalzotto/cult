#include "lzss.h"
#include "lz_window.h"
#include "lz_match_brute.h"
#include "lz_serialize.h"
#include "print.h"

static void* compress(u8* begin, u8* end, lzss_config config, stack_alloc* alloc, file_t debug) {
    lz_window window = {
        .search_begin = begin,
        .lookahead_begin = begin,
        .end = end,
    };
    lz_match_span matches;
    matches.begin = alloc->cursor;
    while (!lz_window_end(window, config.match_size_min)) {
        lz_match match = lz_match_brute(window, config.match_size_max);
        u8* lookahead_next;
        if (lz_match_has_value(match) && lz_match_is_large_enough(match, config.match_size_min)) {
            *(lz_match*)sa_alloc(alloc, sizeof(match)) = match;
            lookahead_next = match.lookahead.end;
        } else {
            lookahead_next = byteoffset(window.lookahead_begin, 1);
        }
        lz_window_advance(&window, lookahead_next, config.window_size_max);
    }
    matches.end = alloc->cursor;

    if (debug) {
        for (lz_match* match = matches.begin; match<matches.end; ++match) {
            print_format(debug, STRING("match: %u bytes\n"), bytesize(match->search.begin, match->search.end));
            // print_string(debug, (string){match->search.begin, match->search.end});
            // print_string(debug, STRING("\n"));
        }
    }
    
    u8* output = lz_serialize(begin, end, matches, alloc);

    sa_move_tail(alloc, output, matches.begin);

    return matches.begin;
}

void* lzss_compress(u8* begin, u8* end, lzss_config config, stack_alloc* alloc, file_t debug) {
    return compress(begin, end, config, alloc, debug);
}

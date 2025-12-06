#include "lzss.h"
#include "lz_window.h"
#include "lz_match_brute.h"
#include "lzss_serialize.h"
#include "lzss_deserialize.h"
#include "print.h"

static void* compress(u8* begin, u8* end, lzss_config config, stack_alloc* alloc, file_t debug) {
    lz_window window = {
        .search_begin = begin,
        .lookahead_begin = begin,
        .end = end,
    };
    lz_match_slice matches;
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
            print_format(debug, STRING("match: %u bytes\n"), bytesize(match->search.begin, match->search.end));;
        }
    }
    
    u8* output = lzss_serialize(begin, end, matches, config.match_size_max, alloc);

    sa_move_tail(alloc, output, matches.begin);

    return matches.begin;
}

static u8* decompress(u8* begin, u8* end, stack_alloc* alloc, file_t debug) {
    return lzss_deserialize(begin, end, alloc, debug);
}

void* lzss_compress(u8* begin, u8* end, lzss_config config, stack_alloc* alloc, file_t debug) {
    return compress(begin, end, config, alloc, debug);
}

void* lzss_decompress(u8* begin, u8* end, stack_alloc* alloc, file_t debug) {
    return decompress(begin, end, alloc, debug);
}

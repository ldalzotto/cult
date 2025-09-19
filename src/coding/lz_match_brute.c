#include "lz_match_brute.h"

static lz_match update_largest_match(lz_match match_largest, lz_match match_current) {
    uptr match_current_size = bytesize(match_current.search.begin, match_current.search.end);
    uptr match_largest_size = bytesize(match_largest.search.begin, match_largest.search.end);
    if (match_current_size > match_largest_size) {
        return match_current;
    } else {
        return match_largest;
    }
}

u8 lz_match_has_value(lz_match match) {
    return bytesize(match.search.begin, match.search.end) > 0;
}

lz_match lz_match_brute(lz_window window) {
    // TODO
    unused(window);
    lz_match match_largest = {
        .search = {window.search_begin, window.search_begin},
        .lookahead = {window.lookahead_begin, window.lookahead_begin},
    };
    u8* lookahead_cursor = window.lookahead_begin;
    u8* search_start = window.search_begin;
    u8* search_cursor = search_start;
    u8 comparison_started = 0;
    while (1) {
        if (lookahead_cursor == window.end ||
            search_cursor == window.lookahead_begin) {
                if (comparison_started) {
                    comparison_started = 0;
                    lz_match match_current = {
                        .search = {search_start, search_cursor},
                        .lookahead = {window.lookahead_begin, lookahead_cursor},
                    };
                    match_largest = update_largest_match(match_largest, match_current);
                }
                break;
        }

        if (*search_cursor == *lookahead_cursor) {
            if (!comparison_started) {
                search_start = search_cursor;
                comparison_started = 1;
            }
            ++search_cursor;
            ++lookahead_cursor;
        } else {
            if (comparison_started) {
                lz_match match_current = {
                    .search = {search_start, search_cursor},
                    .lookahead = {window.lookahead_begin, lookahead_cursor},
                };
                match_largest = update_largest_match(match_largest, match_current);
                lookahead_cursor = window.lookahead_begin;
                comparison_started = 0;
            } else {
                ++search_cursor;
            }
        }
    }

    return match_largest;
}

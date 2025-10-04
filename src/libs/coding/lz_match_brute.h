#ifndef LZ_MATCH_BRUTE
#define LZ_MATCH_BRUTE

#include "lz_window.h"
#include "lzss_config.h"

typedef struct {
    struct {u8* begin; u8* end;} search;
    struct {u8* begin; u8* end;} lookahead;
} lz_match;

typedef struct {
    lz_match* begin; lz_match* end;
} lz_match_slice;

u8 lz_match_has_value(lz_match match);
u8 lz_match_is_large_enough(lz_match match, lzss_match_size_t match_size_min);
lz_match lz_match_brute(lz_window window, lzss_match_size_t match_size_max);

#endif /* LZ_MATCH_BRUTE */

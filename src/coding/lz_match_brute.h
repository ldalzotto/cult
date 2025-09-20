#ifndef LZ_MATCH_BRUTE
#define LZ_MATCH_BRUTE

#include "lz_window.h"

typedef struct {
    struct {u8* begin; u8* end;} search;
    struct {u8* begin; u8* end;} lookahead;
} lz_match;

typedef struct {
    lz_match* begin; lz_match* end;
} lz_match_span;

u8 lz_match_has_value(lz_match match);
lz_match lz_match_brute(lz_window window);

#endif /* LZ_MATCH_BRUTE */

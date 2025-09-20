#ifndef LZ_SERIALIZE_H
#define LZ_SERIALIZE_H

#include "lz_match_brute.h"
#include "stack_alloc.h"

u8* lz_serialize(u8* input_begin, u8* input_end, lz_match_span matches, u8 match_size_max, stack_alloc* alloc);

#endif

#ifndef LZ_SERIALIZE_H
#define LZ_SERIALIZE_H

#include "stack_alloc.h"
#include "lz_match_brute.h"
#include "lzss_config.h"

u8* lz_serialize(u8* input_begin, u8* input_end, lz_match_span matches, lzss_match_size_t match_size_max, stack_alloc* alloc);

#endif

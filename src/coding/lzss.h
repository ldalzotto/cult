#ifndef LZSS_H
#define LZSS_H

#include "stack_alloc.h"
#include "file.h"

typedef struct {
    u8 match_size_min;
    u8 match_size_max;
    u16 window_size_max;
} lzss_config;

void* lzss_compress(u8* begin, u8* end, lzss_config config, stack_alloc* alloc, file_t debug);

#endif

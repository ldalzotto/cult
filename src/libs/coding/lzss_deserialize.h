#ifndef LZ_DESERIALIZE_H
#define LZ_DESERIALIZE_H

#include "stack_alloc.h"
#include "file.h"

u8* lzss_deserialize(u8* compressed_begin, u8* compressed_end, stack_alloc* alloc, file_t debug);

#endif

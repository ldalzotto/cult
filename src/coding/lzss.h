#ifndef LZSS_H
#define LZSS_H

#include "stack_alloc.h"
#include "file.h"

void* lzss_compress(u8* begin, u8* end, stack_alloc* alloc, file_t debug);

#endif

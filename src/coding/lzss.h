#ifndef LZSS_H
#define LZSS_H

#include "stack_alloc.h"

void* lzss_compress(u8* begin, u8* end, stack_alloc* alloc, u8 debug);

#endif

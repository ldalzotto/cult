#ifndef HF_FREQUENCY_H
#define HF_FREQUENCY_H

#include "primitive.h"
#include "stack_alloc.h"

typedef struct {
    u32* begin;
    u32* end;
} hf_frequency_array;

hf_frequency_array hf_frequency(u8* begin, u8* end, stack_alloc* alloc);

#endif

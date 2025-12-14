#ifndef HF_DESERIALIZE_H
#define HF_DESERIALIZE_H

#include "stack_alloc.h"

u8* hf_deserialize(u8* input_begin, u8* input_end, stack_alloc* alloc);

#endif

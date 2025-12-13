#ifndef HF_SERIALIZE_H
#define HF_SERIALIZE_H

#include "hf_code_table.h"
#include "stack_alloc.h"

u8* hf_serialize(u8* input_begin, u8* input_end, hf_code_table* code_table, stack_alloc* alloc);

#endif

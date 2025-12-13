#ifndef HF_CODE_TABLE_H
#define HF_CODE_TABLE_H

#include "primitive.h"

typedef struct {
    u8 symbol;
    u8 code;
    u8 code_bit_count;
} hf_code_entry;

typedef struct {
    hf_code_entry* begin;
    hf_code_entry* end;
} hf_code_table;

#endif

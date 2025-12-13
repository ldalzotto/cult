#ifndef HF_CODE_TABLE_BUILD_H
#define HF_CODE_TABLE_BUILD_H

#include "hf_code_table.h"
#include "hf_tree.h"
#include "stack_alloc.h"

hf_code_table hf_code_table_build(hf_tree* tree, stack_alloc* alloc);
hf_code_table hf_code_table_build_with_noop(hf_code_table* table, stack_alloc* alloc);

#endif

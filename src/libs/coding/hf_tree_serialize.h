#ifndef HF_TREE_SERIALIZE_H
#define HF_TREE_SERIALIZE_H

#include "hf_tree.h"
#include "stack_alloc.h"

void* hf_tree_serialize(hf_tree tree, stack_alloc* alloc);
hf_tree hf_tree_deserialize(void* begin, void* end, stack_alloc* alloc);

#endif

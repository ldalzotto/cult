#ifndef HF_TREE_H
#define HF_TREE_H

#include "primitive.h"

typedef struct {
    u8 symbol;
    u32 frequency;
    struct hf_node *left;
    struct hf_node *right;
} hf_node;

typedef struct {
    hf_node* begin;
    hf_node* end;
} hf_tree;

#endif

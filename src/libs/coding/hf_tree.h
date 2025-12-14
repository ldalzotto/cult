#ifndef HF_TREE_H
#define HF_TREE_H

#include "primitive.h"
#include "meta.h"

typedef struct hf_node hf_node;

struct hf_node {
    u8 symbol;
    u32 frequency;
    hf_node *left;
    hf_node *right;
};

typedef struct {
    hf_node* begin;
    hf_node* end;
} hf_tree;

// Fields for test_point_t
static const field_descriptor hf_node_fields[] = {
    {STR("symbol"), offsetof(hf_node, symbol), &u8_meta},
    {STR("frequency"), offsetof(hf_node, frequency), &u32_meta},
    {STR("left"), offsetof(hf_node, left), &uptr_meta},
    {STR("right"), offsetof(hf_node, right), &uptr_meta},
};

// Meta for test_point_t
static const meta hf_node_meta = {
    .type_name = STR("hf_node"),
    .type_size = sizeof(hf_node),
    .pt = PT_NONE,
    .fields = {
        RANGE(hf_node_fields)
    },
};

static const meta hf_node_array_meta = {
    .type_size = sizeof(hf_node*) * 2,
    .pt = PT_ARRAY,
    .array_element_meta = &hf_node_meta
};

#endif

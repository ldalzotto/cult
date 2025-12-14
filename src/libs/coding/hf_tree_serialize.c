#include "hf_tree_serialize.h"

// TODO improve size efficiency. Example, both indices can be stored on a single byte.

typedef struct {
    u8 symbol;
    u8 left_index;
    u8 right_index;
} hf_entry_serialized;

void* hf_tree_serialize(hf_tree tree, stack_alloc* alloc) {
    void* begin = alloc->cursor;
    for (hf_node* node = tree.begin; node < tree.end; ++node) {
        hf_entry_serialized* s = sa_alloc(alloc, sizeof(*s));
        s->symbol = node->symbol;
        if (node->left) {
            s->left_index = node->left - tree.begin;
        } else {
            s->left_index = 0xFF;
        }
        
        if (node->right) {
            s->right_index = node->right - tree.begin;
        } else {
            s->right_index = 0xFF;
        }
    }

    return begin;
}

hf_tree hf_tree_deserialize(void* begin, void* end, stack_alloc* alloc) {
    struct {hf_entry_serialized* begin; hf_entry_serialized* end;} tree_serialized;
    tree_serialized.begin = begin;
    tree_serialized.end = end;

    hf_tree tree;
    tree.begin = alloc->cursor;

    for (hf_entry_serialized* entry = tree_serialized.begin; 
        entry < (hf_entry_serialized*)tree_serialized.end; 
        entry = byteoffset(entry, sizeof(*entry))) {
            hf_node* node = sa_alloc(alloc, sizeof(*node));
            node->frequency = 0;
            node->symbol = entry->symbol;
            if (entry->left_index != 0xFF) {
                node->left = tree.begin + entry->left_index;
            } else {
                node->left = 0;
            }
            
            if (entry->right_index != 0xFF) {
                node->right = tree.begin + entry->right_index;
            } else {
                node->right = 0;
            }
    }
    
    tree.end = alloc->cursor;
    return tree;
}


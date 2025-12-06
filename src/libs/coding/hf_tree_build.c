#include "hf_tree_build.h"
#include "assert.h"

typedef struct {
    hf_node* begin;
    hf_node* end;
} hf_node_slice;

static hf_node_slice initial_nodes(hf_frequency_array frequency, stack_alloc* alloc) {
    hf_node_slice slice;
    slice.begin = alloc->cursor;
    
    u8 symbol = 0;
    for (u32* f = frequency.begin; f < frequency.end;) {
        if (*f > 0) {
            hf_node* node = sa_alloc(alloc, sizeof(*node));
            node->frequency = *f;
            node->symbol = symbol;
            node->left = 0;
            node->right = 0;
        }

        ++f;
        symbol += 1;
    }

    slice.end = alloc->cursor;
    return slice;
}

static void nodes_sort(hf_node_slice nodes, stack_alloc* alloc) {
    hf_node_slice tmp;
    tmp.begin = sa_alloc(alloc, bytesize(nodes.begin, nodes.end));
    tmp.end = alloc->cursor;
    hf_node* tmp_cursor = tmp.begin;
    
    for (hf_node* input_node = nodes.begin; input_node < nodes.end; ++input_node) {
        hf_node* tmp_insert_position = tmp.begin;
        for (hf_node* tmp_node = tmp.begin; tmp_node < tmp_cursor; ++tmp_node) {
            if (input_node->frequency <= tmp_insert_position->frequency) {
                break;
            }
        }
        
        // Insert
        {
            uptr move_size = bytesize(tmp_insert_position, tmp_cursor);
            sa_move(alloc, tmp_insert_position, tmp_insert_position + 1, move_size);
            *tmp_insert_position = *input_node;
            tmp_cursor += 1;
            debug_assert(tmp_cursor <= tmp.end);
        }
    }

    // Copy the tmp array to the input array
    sa_copy(alloc, tmp.begin, nodes.begin, bytesize(nodes.begin, nodes.end));
    sa_free(alloc, tmp.begin);
}

hf_tree hf_tree_build(hf_frequency_array frequency, stack_alloc* alloc) {
    hf_node_slice initial = initial_nodes(frequency, alloc);
    // Sort the initial nodes
    nodes_sort(initial, alloc);

    // TODO
    hf_tree tree;
    tree.begin = alloc->cursor;
    tree.end = alloc->cursor;
    return tree;
}

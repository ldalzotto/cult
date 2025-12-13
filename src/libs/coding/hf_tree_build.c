#include "hf_tree_build.h"
#include "assert.h"
#include "print.h"

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

static void node_insert(hf_node* end, hf_node* position, hf_node value, stack_alloc* alloc) {
    uptr move_size = bytesize(position, end);
    sa_move(alloc, position, position + 1, move_size);
    *position = value;
}

static void nodes_sort(hf_node_slice nodes, stack_alloc* alloc) {
    hf_node_slice tmp;
    tmp.begin = sa_alloc(alloc, bytesize(nodes.begin, nodes.end));
    tmp.end = alloc->cursor;
    hf_node* tmp_cursor = tmp.begin;
    
    for (hf_node* input_node = nodes.begin; input_node < nodes.end; ++input_node) {
        hf_node* tmp_insert_position = tmp.begin;
        for (; tmp_insert_position < tmp_cursor;++tmp_insert_position) {
            if (input_node->frequency >= tmp_insert_position->frequency) {
                break;
            }
        }
        
        // Insert
        {
            node_insert(tmp_cursor, tmp_insert_position, *input_node, alloc);
            tmp_cursor += 1;
            debug_assert(tmp_cursor <= tmp.end);
        }
    }

    for (hf_node* tmp_node = tmp.begin; tmp_node < tmp.end; ++tmp_node) {
        u32 value = tmp_node->frequency;
        unused(value);
    }

    // Copy the tmp array to the input array
    sa_copy(alloc, tmp.begin, nodes.begin, bytesize(nodes.begin, nodes.end));
    sa_free(alloc, tmp.begin);
}

static hf_node_slice nodes_merge(hf_node_slice nodes, stack_alloc* alloc) {
    debug_assert(nodes.end == alloc->cursor);
    print_format(file_stdout(), STRING("%m\n"), &hf_node_array_meta, &nodes);
    
    hf_node* cursor = nodes.end - 1;
    while (1) {
        if (cursor <= nodes.begin) {
            break;
        }

        // peek the last two node
        hf_node* cursor_before = cursor - 1;

        // merge them
        hf_node merged_node;
        merged_node.frequency = cursor->frequency + cursor_before->frequency;
        merged_node.left = cursor_before + 1; // It is going to be inserted
        merged_node.right = cursor + 1; // It is going to be inserted
        
        // insert the node in the correct position at the sorted array
        hf_node* insert_position = cursor_before;
        {
            hf_node* node = cursor;
            while (node->frequency <= merged_node.frequency && node > nodes.begin) {
                insert_position = node - 1;
                node -= 1;
            }
        }

        debug_assert(insert_position <= cursor);
        debug_assert(insert_position <= cursor_before);
        
        sa_alloc(alloc, sizeof(hf_node));
        node_insert(nodes.end, insert_position, merged_node, alloc);
        nodes.end += 1;
        for (hf_node* to_offset = insert_position + 1; to_offset < nodes.end; ++to_offset) {
            if (to_offset->left) {
                to_offset->left += 1;
            }
            if (to_offset->right) {
                to_offset->right += 1;
            }
        }
        
        cursor -= 1; // Because we have inserted a node before.
    }
    
    print_format(file_stdout(), STRING("%m\n"), &hf_node_array_meta, &nodes);
    return  nodes;
}

hf_tree hf_tree_build(hf_frequency_array frequency, stack_alloc* alloc) {
    hf_node_slice initial = initial_nodes(frequency, alloc);
    // Sort the initial nodes
    nodes_sort(initial, alloc);
    initial = nodes_merge(initial, alloc);

    hf_tree tree;
    tree.begin = initial.begin;
    tree.end = initial.end;
    return tree;
}

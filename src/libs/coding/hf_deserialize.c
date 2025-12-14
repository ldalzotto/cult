#include "hf_deserialize.h"
#include "hf_tree_serialize.h"
#include "bit.h"

static void hf_tree_decode(hf_tree* tree, 
        u8** input_pointer, u8* input_bit_index,
        u8** buffer_pointer) {

    hf_node* node = tree->begin;
    while (1) {
        if (!node->left && !node->right) {
            break;
        }
        u8 bit_value = bit_get(**input_pointer, *input_bit_index);
        *input_bit_index += 1;
        if (*input_bit_index == 8) {
            *input_bit_index = 0;
            *input_pointer += 1;
        }

        if (bit_value == 0) {
            node = node->left;
        } else {
            node = node->right;
        }
    }

    **buffer_pointer = node->symbol;
    *buffer_pointer += 1;
}

u8* hf_deserialize(u8* input_begin, u8* input_end, stack_alloc* alloc) {
    void* begin = alloc->cursor;
    u8* cursor = input_begin;
    u16 table_size = *(u16*)cursor;
    cursor = byteoffset(cursor, sizeof(table_size));
    u8 last_bit_padding = *(u8*)cursor;
    cursor = byteoffset(cursor, sizeof(last_bit_padding));
    
    hf_tree tree = hf_tree_deserialize(cursor, byteoffset(cursor, table_size), alloc);
    cursor = byteoffset(cursor, table_size);

    void* deserialization_begin = alloc->cursor;

    u8* input = cursor;
    u8 input_bit_index = 0;
    u8* output = alloc->cursor;
    while (1) {
        const u8 isLastByte = input == input_end - 1;
        if (isLastByte && input_bit_index == last_bit_padding) {
            break;
        }
        debug_assert(input < input_end);
        hf_tree_decode(&tree, &input, &input_bit_index, &output);
    }

    alloc->cursor = output;
    
    sa_move_tail(alloc, deserialization_begin, begin);

    return begin;
}

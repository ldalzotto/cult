#include "hf_serialize.h"
#include "hf_code_table_build.h"
#include "hf_tree_serialize.h"
#include "bit.h"

u8* hf_serialize(u8* input_begin, u8* input_end, hf_tree tree, hf_code_table* code_table, stack_alloc* alloc) {
    void* begin = alloc->cursor;
    hf_code_table table_with_noop = hf_code_table_build_with_noop(code_table, alloc);

    void* serialization_begin = alloc->cursor;
    // Serialize tree
    u16* tree_size = sa_alloc(alloc, sizeof(*tree_size));
    u8* last_bit_padding = sa_alloc(alloc, sizeof(*last_bit_padding));

    void* hf_tree_begin = hf_tree_serialize(tree, alloc);
    *tree_size = bytesize(hf_tree_begin, alloc->cursor);
    
    // TODO: store the last byte padding
    
    // Encode text
    u8 out_bit_cursor = 8;
    u8* out_cursor = alloc->cursor;
    
    for (
        u8* input_cursor = input_begin;
        input_cursor < input_end;
        ++input_cursor
    ) {
        hf_code_entry* entry = table_with_noop.begin + *input_cursor;
        for (u8 bit_index = 0; bit_index < entry->code_bit_count; ++bit_index) {
            if (out_bit_cursor == 8) {
                out_cursor = sa_alloc(alloc, 1);
                out_bit_cursor = 0;
            }
            u8 bit_value = bit_get(entry->code, bit_index);
            *out_cursor = bit_write(*out_cursor, out_bit_cursor, bit_value);
            out_bit_cursor += 1;
        }
    if (out_bit_cursor == 8) {
        *last_bit_padding = 0;
    } else {
        *last_bit_padding = out_bit_cursor;
    }

    sa_move_tail(alloc, serialization_begin, begin);
    return begin;
}

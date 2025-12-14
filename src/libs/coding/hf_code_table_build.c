#include "hf_code_table_build.h"
#include "bit.h"
#include "hf_symbol.h"

hf_code_table hf_code_table_build(hf_tree* tree, stack_alloc* alloc) {
    void* begin = alloc->cursor;
    typedef struct {
        hf_node* node;
        u8 current_value;
        u8 bit_count;
    } entry;
    struct {entry* begin; entry* cursor; entry* end;} stack;
    stack.begin = sa_alloc(alloc, sizeof(entry) * hf_symbol_max);
    stack.cursor = stack.begin;
    stack.end = alloc->cursor;

    *stack.begin = (entry){
        .node = tree->begin,
        .current_value = 0,
        .bit_count = 0,
    };
    stack.cursor += 1;

    void* table_begin = alloc->cursor;

    entry* cursor = stack.cursor;
    while (cursor != stack.begin) {
        cursor -= 1;
        entry e = *cursor;

        // push others
        if (e.node->left && e.node->right) {
            *cursor = (entry){
                .node = e.node->left,
                .current_value = bit_write(e.current_value, e.bit_count, 0),
                .bit_count = e.bit_count + 1,
            };
            cursor += 1;
            *cursor = (entry){
                .node = e.node->right,
                .current_value = bit_write(e.current_value, e.bit_count, 1),
                .bit_count = e.bit_count + 1,
            };
            cursor += 1;
        } else {
            hf_code_entry* code = sa_alloc(alloc, sizeof(hf_code_entry));
            code->code = e.current_value;
            code->symbol = e.node->symbol;
            code->code_bit_count = e.bit_count;
        }
    }

    sa_move_tail(alloc, table_begin, begin);
    
    return (hf_code_table) {
        begin,
        alloc->cursor,
    };
}

hf_code_table hf_code_table_build_with_noop(hf_code_table* table, stack_alloc* alloc) {
    hf_code_table table_with_noop;
    table_with_noop.begin = sa_alloc(alloc, sizeof(*table_with_noop.begin) * hf_symbol_max);
    table_with_noop.end = alloc->cursor;

    u8 symbol = 0;
    for (hf_code_entry* entry = table_with_noop.begin; entry < table_with_noop.end; ++entry) {
        hf_code_entry* table_entry_to_copy = 0;
        for (hf_code_entry* table_entry = table->begin; table_entry < table->end; ++table_entry) {
            if (table_entry->symbol == symbol) {
                table_entry_to_copy = table_entry;
                break;
            }
        }
        if (table_entry_to_copy != 0) {
            *entry = *table_entry_to_copy;
        } else {
            *entry = (hf_code_entry) {
                .code = 0,
                .code_bit_count = 0,
                .symbol = 0,
            };
        }
        symbol += 1;
    }
    return table_with_noop;
}

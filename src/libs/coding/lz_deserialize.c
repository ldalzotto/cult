#include "lz_deserialize.h"
#include "print.h"
#include "assert.h"
#include "lz_bit_types.h"
#include "bit.h"

static item_type fetch_item_type(item_type_bit_state* bit_state, void** out_cursor) {
    if (bit_state->bit_index == item_type_bit_count) {
        bit_state->value = *out_cursor;
        *out_cursor = byteoffset(bit_state->value, 1);
        bit_state->bit_index = 0;
    }
    item_type b = bit_get(*bit_state->value, bit_state->bit_index);
    bit_state->bit_index += 1;
    return b;
}

u8* lz_deserialize(u8* compressed_begin, u8* compressed_end, stack_alloc* alloc, file_t debug) {
    u8* output = alloc->cursor;
    u8* current = compressed_begin;
    item_type_bit_state bit_state = {.bit_index = item_type_bit_count, .value = compressed_begin};
    while (current < compressed_end) {
        item_type type = fetch_item_type(&bit_state, (void**)&current);
        if (type == LITERAL) {
            u8 size = *(u8*)current;
            current = byteoffset(current, sizeof(size));
            u8* data = sa_alloc(alloc, size);
            sa_copy(alloc, current, data, size);
            current = byteoffset(current, size);
            if (debug) {
                print_format(debug, STRING("%s\n"), (string){output, alloc->cursor});
            }
        } else if (type == MATCH) {
            u16 offset = *(u16*)current;
            current += sizeof(u16);
            u8 length = *current++;
            // Copy from offset back in output
            u8* source = (u8*)alloc->cursor - offset;
            debug_assert(source >= output);
            u8* data = sa_alloc(alloc, length);
            sa_copy(alloc, source, data, length);
        }
    }

    
    return output;
}

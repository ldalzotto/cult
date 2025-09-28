#include "lz_deserialize.h"
#include "print.h"
#include "assert.h"
#include "lz_bit_types.h"

u8* lz_deserialize(u8* compressed_begin, u8* compressed_end, stack_alloc* alloc, file_t debug) {
    u8* output = alloc->cursor;
    u8* current = compressed_begin;

    while (current < compressed_end) {
        item_type type = *(item_type*)current;
        current = byteoffset(current, sizeof(item_type));
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

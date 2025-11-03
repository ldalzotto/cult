#include "lz_serialize.h"
#include "assert.h"
#include "lz_bit_types.h"
#include "bit.h"

static void allocate_item_type(stack_alloc* alloc, item_type_bit_state* bit_state, item_type value) {
    if (bit_state->bit_index == item_type_bit_count) {
        bit_state->bit_index = 0;
        bit_state->value = sa_alloc(alloc, 1);
    }
    *bit_state->value = bit_write(*bit_state->value, bit_state->bit_index, value);
    bit_state->bit_index += 1;
}

static void allocate_litteral(stack_alloc* alloc, item_type_bit_state* bit_state,  u8* begin, u8* end) {
    debug_assert(begin != end);
    allocate_item_type(alloc, bit_state, LITERAL);
    uptr size = bytesize(begin, end);
    *(u8*)sa_alloc(alloc, sizeof(u8)) = size;
    u8* data = sa_alloc(alloc, size);
    sa_copy(alloc, begin, data, size);
}

static void allocate_and_split_litteral(stack_alloc* alloc, item_type_bit_state* bit_state, lzss_match_size_t match_size_max, u8* begin, u8* end) {
    u8* current = begin;
    while (current < end) {
        uptr remaining = end - current;
        lzss_match_size_t chunk_size = remaining < match_size_max ? remaining : match_size_max;

        allocate_litteral(alloc, bit_state, current, byteoffset(current, chunk_size));
        current += chunk_size;
    }
}

static void allocate_match(stack_alloc* alloc, item_type_bit_state* bit_state, lz_match* match) {
    allocate_item_type(alloc, bit_state, MATCH);
    uptr offset = match->lookahead.begin - match->search.begin;
    *(u16*)sa_alloc(alloc, sizeof(u16)) = offset;
    uptr length = match->search.end - match->search.begin;
    *(u8*)sa_alloc(alloc, sizeof(u8)) = length;
}

u8* lz_serialize(u8* input_begin, u8* input_end, lz_match_slice matches, lzss_match_size_t match_size_max, stack_alloc* alloc) {
    u8* output = alloc->cursor;
    u8* current = input_begin;
    item_type_bit_state bit_state = {.bit_index = item_type_bit_count, .value = 0};
    for (lz_match* m = matches.begin; m != matches.end; ++m) {
        // Output literals before match
        allocate_and_split_litteral(alloc, &bit_state, match_size_max, current, m->lookahead.begin);
        // Output match
        allocate_match(alloc, &bit_state, m);
        current = m->lookahead.end;
    }
    // Output remaining literals
    allocate_and_split_litteral(alloc, &bit_state, match_size_max, current, input_end);

    return output;
}

#include "lz_serialize.h"
#include "assert.h"
#include "lz_bit_types.h"

typedef lz_match match;

static void allocate_litteral(stack_alloc* alloc, u8* begin, u8* end) {
    debug_assert(begin != end);
    *(item_type*)sa_alloc(alloc, sizeof(item_type)) = LITERAL;
    uptr size = bytesize(begin, end);
    *(u8*)sa_alloc(alloc, sizeof(u8)) = size;
    u8* data = sa_alloc(alloc, size);
    sa_copy(alloc, begin, data, size);
}

static void allocate_and_split_litteral(stack_alloc* alloc, lzss_match_size_t match_size_max, u8* begin, u8* end) {
    u8* current = begin;
    while (current < end) {
        uptr remaining = end - current;
        lzss_match_size_t chunk_size = remaining < match_size_max ? remaining : match_size_max;

        allocate_litteral(alloc, current, byteoffset(current, chunk_size));
        current += chunk_size;
    }
}

static void allocate_match(stack_alloc* alloc, match* match) {
    *(item_type*)sa_alloc(alloc, sizeof(item_type)) = MATCH;
    uptr offset = match->lookahead.begin - match->search.begin;
    *(u16*)sa_alloc(alloc, sizeof(u16)) = offset;
    uptr length = match->search.end - match->search.begin;
    *(u8*)sa_alloc(alloc, sizeof(u8)) = length;
}

u8* lz_serialize(u8* input_begin, u8* input_end, lz_match_slice matches, lzss_match_size_t match_size_max, stack_alloc* alloc) {
    u8* output = alloc->cursor;
    u8* current = input_begin;
    for (lz_match* m = matches.begin; m != matches.end; ++m) {
        // Output literals before match
        allocate_and_split_litteral(alloc, match_size_max, current, m->lookahead.begin);
        // Output match
        allocate_match(alloc, m);
        current = m->lookahead.end;
    }
    // Output remaining literals
    allocate_and_split_litteral(alloc, match_size_max, current, input_end);

    return output;
}

#include "hf_frequency.h"

hf_frequency_array hf_frequency(u8* begin, u8* end, stack_alloc* alloc) {
    const u8 u8_size = sizeof(u8);
    hf_frequency_array array;
    array.begin = alloc->cursor;
    array.end = sa_alloc(alloc, u8_size * sizeof(*array.begin));

    // Initialize
    for (u32* entry = array.begin; entry < array.end; ++entry) {
        *entry = 0;
    }

    for (u8* cursor = begin; cursor < end; ++cursor) {
        u32* entry = array.begin + *cursor;
        *entry += 1;
    }

    return array;
}

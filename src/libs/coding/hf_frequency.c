#include "hf_frequency.h"

hf_frequency_array hf_frequency(u8* begin, u8* end, stack_alloc* alloc) {
    const u8 u8_max = 0xFF;
    hf_frequency_array array;
    array.begin = sa_alloc(alloc, u8_max * sizeof(*array.begin));
    array.end = alloc->cursor;

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

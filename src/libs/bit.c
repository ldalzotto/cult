#include "bit.h"
#include "assert.h"

/* Mask for a single bit (index 0..7). */
#define BIT_MASK(index) ((u8)(1u << (index)))

/* Read the bit at `index` from `b` (returns 0 or 1). */
u8 bit_get(u8 b, u8 index) {
    debug_assert(index < 8);
    return (u8)((b >> index) & 1u);
}

/* Set a bit (pointer variant). */
u8 bit_set(u8 b, u8 index) {
    debug_assert(index < 8);
    return (u8)(b | BIT_MASK(index));
}

/* Clear a bit (pointer variant). */
u8 bit_clear(u8 b, u8 index) {
    debug_assert(index < 8);
    return (u8)(b & (u8)~BIT_MASK(index));
}

/* Toggle a bit (pointer variant). */
u8 bit_toggle(u8 b, u8 index) {
    debug_assert(index < 8);
    return (u8)(b ^ BIT_MASK(index));
}

/* Write a bit (pointer variant). `value` treated as boolean (0 -> clear, non-zero -> set). */
u8 bit_write(u8 b, u8 index, u8 value) {
    debug_assert(index < 8);
    if (value)
        return bit_set(b, index);
    else
        return bit_clear(b, index);
}

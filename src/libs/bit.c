#include "bit.h"
#include "assert.h"

/* Mask for a single bit (index 0..7). */
#define BIT_MASK(index) ((u8)(1u << (index)))

/* Read the bit at `index` from `b` (returns 0 or 1). */
u8 BIT_GET(u8 b, u8 index) {
    debug_assert(index < 8);
    return (u8)((b >> index) & 1u);
}

/* Set a bit (pointer variant). */
void BIT_SET(u8 *b, u8 index) {
    debug_assert(b != (u8*)0);
    debug_assert(index < 8);
    *b = (u8)(*b | BIT_MASK(index));
}

/* Clear a bit (pointer variant). */
void BIT_CLEAR(u8 *b, u8 index) {
    debug_assert(b != (u8*)0);
    debug_assert(index < 8);
    *b = (u8)(*b & (u8)~BIT_MASK(index));
}

/* Toggle a bit (pointer variant). */
void BIT_TOGGLE(u8 *b, u8 index) {
    debug_assert(b != (u8*)0);
    debug_assert(index < 8);
    *b = (u8)(*b ^ BIT_MASK(index));
}

/* Write a bit (pointer variant). `value` treated as boolean (0 -> clear, non-zero -> set). */
void BIT_WRITE(u8 *b, u8 index, u8 value) {
    debug_assert(b != (u8*)0);
    debug_assert(index < 8);
    if (value)
        BIT_SET(b, index);
    else
        BIT_CLEAR(b, index);
}

#if 0
/* Value-returning variants (operate on and return a u8) */

/* Return byte with bit set */
static inline u8 bit_set_val(u8 b, u8 index) {
    assert(index < 8);
    return (u8)(b | BIT_MASK(index));
}

/* Return byte with bit cleared */
static inline u8 bit_clear_val(u8 b, u8 index) {
    assert(index < 8);
    return (u8)(b & (u8)~BIT_MASK(index));
}

/* Return byte with bit toggled */
static inline u8 bit_toggle_val(u8 b, u8 index) {
    assert(index < 8);
    return (u8)(b ^ BIT_MASK(index));
}

/* Return byte with bit written to `value` (0 or non-zero) */
static inline u8 bit_write_val(u8 b, u8 index, u8 value) {
    assert(index < 8);
    return value ? bit_set_val(b, index) : bit_clear_val(b, index);
}


#endif

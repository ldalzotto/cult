
#ifndef BIT_H
#define BIT_H

#include "primitive.h"

/* Read the bit at `index` from `b` (returns 0 or 1). */
u8 bit_get(u8 b, u8 index);

/* Set a bit. */
u8 bit_set(u8 b, u8 index);

/* Clear a bit. */
u8 bit_clear(u8 b, u8 index);

/* Toggle a bit. */
u8 bit_toggle(u8 b, u8 index);

/* Write a bit. `value` treated as boolean (0 -> clear, non-zero -> set). */
u8 bit_write(u8 b, u8 index, u8 value);

#endif /* BIT_H */

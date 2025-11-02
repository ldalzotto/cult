
#ifndef BIT_H
#define BIT_H

#include "primitive.h"

/* Read the bit at `index` from `b` (returns 0 or 1). */
u8 BIT_GET(u8 b, u8 index);

/* Set a bit. */
u8 BIT_SET(u8 b, u8 index);

/* Clear a bit. */
u8 BIT_CLEAR(u8 b, u8 index);

/* Toggle a bit. */
u8 BIT_TOGGLE(u8 b, u8 index);

/* Write a bit. `value` treated as boolean (0 -> clear, non-zero -> set). */
u8 BIT_WRITE(u8 b, u8 index, u8 value);

#endif /* BIT_H */

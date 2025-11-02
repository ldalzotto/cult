
#ifndef BIT_H
#define BIT_H

#include "primitive.h"

/* Read the bit at `index` from `b` (returns 0 or 1). */
u8 BIT_GET(u8 b, u8 index);

/* Set a bit (pointer variant). */
void BIT_SET(u8 *b, u8 index);

/* Clear a bit (pointer variant). */
void BIT_CLEAR(u8 *b, u8 index);

/* Toggle a bit (pointer variant). */
void BIT_TOGGLE(u8 *b, u8 index);

/* Write a bit (pointer variant). `value` treated as boolean (0 -> clear, non-zero -> set). */
void BIT_WRITE(u8 *b, u8 index, u8 value);

#endif /* BIT_H */

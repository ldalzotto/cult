#ifndef LZ_BIT_TYPES_H
#define LZ_BIT_TYPES_H

#include "primitive.h"

// TODO: For size optimization, we can write this as an 8 bit field
typedef u8 item_type;
static const item_type LITERAL = 0;
static const item_type MATCH = 1;
STATIC_ASSERT(sizeof(item_type) == 1);

#endif

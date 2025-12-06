#ifndef LZ_BIT_TYPES_H
#define LZ_BIT_TYPES_H

#include "primitive.h"

typedef u8 item_type;
static const item_type LITERAL = 0;
static const item_type MATCH = 1;
STATIC_ASSERT(sizeof(item_type) == 1);

// Used for size optimisation. It stores 8 item_type flag on a single but
typedef struct {
    u8* value;
    u8 bit_index;
} item_type_bit_state;

static const u8 item_type_bit_count = 8;

#endif

#ifndef WIN_H
#define WIN_H

#include "../primitive.h"

/**
 * @brief Simple event structure
 */
typedef struct {
    i32 type;
} win_event;

typedef struct {
    void* begin;
    void* end;
} win_buffer;

#endif /* WIN_H */

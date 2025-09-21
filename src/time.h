#ifndef TIME_H
#define TIME_H

#include "stack_alloc.h"
#include "primitive.h"


// Allocates and returns formatted "HH:MM:SS" string using stack allocator.
// Caller must not free manually, but can reset/free via sa_free or sa_deinit.
char* time_str(u64 microseconds, stack_alloc* alloc);

// Allocates and returns formatted "HH:MM:SS.UUUUUU" string using stack allocator (microsecond precision)
char* time_str_us(u64 microseconds, stack_alloc* alloc);


#endif // TIME_H

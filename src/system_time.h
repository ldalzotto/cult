#ifndef SYSTEM_TIME_H
#define SYSTEM_TIME_H

#include "primitive.h"

// Returns current time in milliseconds since Unix epoch
u64 sys_time_ms(void);

// Returns current time in seconds since Unix epoch
u64 sys_time_s(void);

// Returns current time in microseconds since Unix epoch
u64 sys_time_us(void);

#endif // SYSTEM_TIME_H

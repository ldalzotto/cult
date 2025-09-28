#include "system_time.h"
#include <sys/syscall.h>
#include <unistd.h>
#include <linux/time.h>  // struct timespec

// Milliseconds since epoch
u64 sys_time_ms(void) {
    struct timespec ts;
    syscall(SYS_clock_gettime, CLOCK_REALTIME, &ts);
    return (u64)ts.tv_sec * 1000ULL + (u64)ts.tv_nsec / 1000000ULL;
}

// Seconds since epoch
u64 sys_time_s(void) {
    struct timespec ts;
    syscall(SYS_clock_gettime, CLOCK_REALTIME, &ts);
    return (u64)ts.tv_sec;
}

// Microseconds since epoch
u64 sys_time_us(void) {
    struct timespec ts;
    syscall(SYS_clock_gettime, CLOCK_REALTIME, &ts);
    return (u64)ts.tv_sec * 1000000ULL + (u64)ts.tv_nsec / 1000ULL;
}

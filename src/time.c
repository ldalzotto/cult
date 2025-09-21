#include "time.h"
#include "print.h"

typedef struct {
    i32 hour;
    i32 minute;
    i32 second;
    u32 microsecond; // 0 if not needed
} sys_time_components;

// Convert microseconds since epoch to HH:MM:SS + µs
static sys_time_components epoch_to_components(u64 microseconds) {
    sys_time_components st;

    u64 total_seconds = microseconds / 1000000ULL;
    st.microsecond = (u32)(microseconds % 1000000ULL);

    u64 seconds_in_day = total_seconds % 86400ULL;
    st.hour = (i32)(seconds_in_day / 3600ULL);
    st.minute = (i32)((seconds_in_day % 3600ULL) / 60ULL);
    st.second = (i32)(seconds_in_day % 60ULL);

    return st;
}

// Allocates "HH:MM:SS" string from stack allocator (UTC)
char* time_str(u64 microseconds, stack_alloc* alloc) {
    sys_time_components st = epoch_to_components(microseconds);

    return (char*)print_format_to_buffer(
        alloc,
        STRING("%d:%d:%d"),
        st.hour,
        st.minute,
        st.second
    );
}

// Allocates "HH:MM:SS.UUUUUU" string from stack allocator (UTC, microsecond precision)
char* time_str_us(u64 microseconds, stack_alloc* alloc) {
    sys_time_components st = epoch_to_components(microseconds);

    return (char*)print_format_to_buffer(
        alloc,
        STRING("%d:%d:%d.%d"),
        st.hour,
        st.minute,
        st.second,
        st.microsecond
    );
}

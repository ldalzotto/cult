#include "time.h"
#include "print.h"
#include "assert.h"

typedef struct {
    i32 hour;
    i32 minute;
    i32 second;
    u32 microsecond;
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

// ---------------- FPS TICKER ----------------

void fps_ticker_init(fps_ticker* ticker, u64 preferred_frame_us, u64 start_time_us) {
    ticker->preferred_frame_us = preferred_frame_us;
    ticker->last_time_us = start_time_us;
}

u32 fps_ticker_update(fps_ticker* ticker, u64 current_time_us) {
    debug_assert(current_time_us >= ticker->last_time_us);

    u64 delta_us = current_time_us - ticker->last_time_us;

    u32 frames = (u32)(delta_us / ticker->preferred_frame_us);
    ticker->last_time_us += (frames * ticker->preferred_frame_us);

    return frames;
}

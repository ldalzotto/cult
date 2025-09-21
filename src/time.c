#include "time.h"
#include "assert.h"

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

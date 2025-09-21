#ifndef TIME_H
#define TIME_H

#include "primitive.h"

typedef struct fps_ticker {
    u64 preferred_frame_us;   // target duration per frame
    u64 last_time_us;         // last tick time in microseconds
} fps_ticker;

// Initializes an FPS ticker
void fps_ticker_init(fps_ticker* ticker, u64 preferred_frame_us, u64 start_time_us);

// Updates ticker with current time and returns number of frames that should be processed
u32 fps_ticker_update(fps_ticker* ticker, u64 current_time_us);

#endif // TIME_H

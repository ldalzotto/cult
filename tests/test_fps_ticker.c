#include "test_fps_ticker.h"
#include "stack_alloc.h"
#include "mem.h"
#include "time.h"
#include "primitive.h"
#include "print.h"

// Test that FPS ticker generates frames correctly and that estimated next frame time is correct
static void test_fps_ticker_basic(test_context* t) {
    // Allocate memory for stack allocator (not strictly needed for ticker)
    const uptr stack_size = 1024;
    void* memory = mem_map(stack_size);
    stack_alloc alloc;
    sa_init(&alloc, memory, byteoffset(memory, stack_size));

    fps_ticker ticker;
    const u64 frame_us = 10000; // 10 ms per frame (~100 FPS)
    const u64 start_time = 0;   // simulated start at 0 \u00b5s

    fps_ticker_init(&ticker, frame_us, start_time);

    // Initial estimated next frame should be start + preferred_frame_us
    TEST_ASSERT_EQUAL(t, fps_ticker_estimated_next_frame(&ticker), start_time + frame_us);

    // Simulate time advancing
    u64 time_us = 0;

    // No time passed yet, should produce 0 frames and estimated next frame unchanged
    u32 frames = fps_ticker_update(&ticker, time_us);
    TEST_ASSERT_EQUAL(t, frames, 0);
    TEST_ASSERT_EQUAL(t, fps_ticker_estimated_next_frame(&ticker), start_time + frame_us);

    // Advance 5 ms -> still 0 frames
    time_us += 5000;
    frames = fps_ticker_update(&ticker, time_us);
    TEST_ASSERT_EQUAL(t, frames, 0);
    TEST_ASSERT_EQUAL(t, fps_ticker_estimated_next_frame(&ticker), start_time + frame_us);

    // Advance another 5 ms -> total 10 ms -> 1 frame
    time_us += 5000;
    frames = fps_ticker_update(&ticker, time_us);
    TEST_ASSERT_EQUAL(t, frames, 1);
    // After consuming one frame, last_time becomes 10000 -> next estimated is 20000
    TEST_ASSERT_EQUAL(t, fps_ticker_estimated_next_frame(&ticker), 20000ULL);

    // Advance 30 ms -> should generate 3 frames (from 10000 -> 40000)
    time_us += 30000;
    frames = fps_ticker_update(&ticker, time_us);
    TEST_ASSERT_EQUAL(t, frames, 3);
    // last_time is now 40000 -> next estimated is 50000
    TEST_ASSERT_EQUAL(t, fps_ticker_estimated_next_frame(&ticker), 50000ULL);

    // Advance 15 ms -> 1 frame + 5 ms leftover (from 40000 -> 55000; consume 1 frame -> last_time 50000)
    time_us += 15000;
    frames = fps_ticker_update(&ticker, time_us);
    TEST_ASSERT_EQUAL(t, frames, 1);
    // last_time is now 50000 -> next estimated is 60000
    TEST_ASSERT_EQUAL(t, fps_ticker_estimated_next_frame(&ticker), 60000ULL);

    // Immediately call again without advancing time -> 0 frames, estimated unchanged
    frames = fps_ticker_update(&ticker, time_us);
    TEST_ASSERT_EQUAL(t, frames, 0);
    TEST_ASSERT_EQUAL(t, fps_ticker_estimated_next_frame(&ticker), 60000ULL);

    // Advance 5 ms -> reach 60000 -> 1 frame
    time_us += 5000;
    frames = fps_ticker_update(&ticker, time_us);
    TEST_ASSERT_EQUAL(t, frames, 1);
    TEST_ASSERT_EQUAL(t, fps_ticker_estimated_next_frame(&ticker), 70000ULL);

    sa_deinit(&alloc);
    mem_unmap(memory, stack_size);

    TEST_ASSERT_TRUE(t, 1);
}

void test_fps_ticker_module(test_context* t) {
    REGISTER_TEST(t, "fps_ticker_basic", test_fps_ticker_basic);
}

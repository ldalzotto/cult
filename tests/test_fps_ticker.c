#include "test_fps_ticker.h"
#include "stack_alloc.h"
#include "mem.h"
#include "time.h"
#include "primitive.h"
#include "print.h"

// Test that FPS ticker generates frames correctly
static void test_fps_ticker_basic(test_context* t) {
    // Allocate memory for stack allocator (not strictly needed for ticker)
    const uptr stack_size = 1024;
    void* memory = mem_map(stack_size);
    stack_alloc alloc;
    sa_init(&alloc, memory, byteoffset(memory, stack_size));

    fps_ticker ticker;
    const u64 frame_us = 10000; // 10 ms per frame (~100 FPS)
    const u64 start_time = 0;   // simulated start at 0 µs

    fps_ticker_init(&ticker, frame_us, start_time);

    // Simulate time advancing
    u64 time_us = 0;

    // No time passed yet, should produce 0 frames
    u32 frames = fps_ticker_update(&ticker, time_us);
    TEST_ASSERT_EQUAL(t, frames, 0);

    // Advance 5 ms -> still 0 frames
    time_us += 5000;
    frames = fps_ticker_update(&ticker, time_us);
    TEST_ASSERT_EQUAL(t, frames, 0);

    // Advance another 5 ms -> total 10 ms -> 1 frame
    time_us += 5000;
    frames = fps_ticker_update(&ticker, time_us);
    TEST_ASSERT_EQUAL(t, frames, 1);

    // Advance 30 ms -> should generate 3 frames
    time_us += 30000;
    frames = fps_ticker_update(&ticker, time_us);
    TEST_ASSERT_EQUAL(t, frames, 3);

    // Advance 15 ms -> 1 frame + 5 ms leftover
    time_us += 15000;
    frames = fps_ticker_update(&ticker, time_us);
    TEST_ASSERT_EQUAL(t, frames, 1);

    // Immediately call again without advancing time -> 0 frames
    frames = fps_ticker_update(&ticker, time_us);
    TEST_ASSERT_EQUAL(t, frames, 0);

    time_us += 5000;
    frames = fps_ticker_update(&ticker, time_us);
    TEST_ASSERT_EQUAL(t, frames, 1);

    sa_deinit(&alloc);
    mem_unmap(memory, stack_size);

    TEST_ASSERT_TRUE(t, 1);
}

void test_fps_ticker_module(test_context* t) {
    REGISTER_TEST(t, "fps_ticker_basic", test_fps_ticker_basic);
}

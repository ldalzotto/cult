#include "mem.h"
#include "stack_alloc.h"
#include "primitive.h"
#include "network/https/https_request.h"
#include "test_network_https.h"
#include "print.h"

/* Test https_request_sync when tcp_connect fails (e.g. host cannot be resolved).
   The function should cleanly return the initial allocator cursor (no crash). */
static void test_https_request_returns_initial_pointer_on_failed_resolve(test_context* t) {
    uptr size = 64 * 1024;
    void* pointer = mem_map(size);
    TEST_ASSERT_EQUAL(t, (int)(pointer != 0), 1);
    stack_alloc alloc;
    sa_init(&alloc, pointer, byteoffset(pointer, size));

    /* Save begin cursor */
    void* begin = alloc.cursor;

    u8 host_buf[] = "127.0.0.1";
    u8 port_buf[] = "443";
    u8 req_buf[] = "GET / HTTP/1.0\r\nHost: no-such-hostname-for-unit-test.invalid\r\n\r\n";

    u8_slice host = { host_buf, byteoffset(host_buf, mem_cstrlen(host_buf)) };
    u8_slice port_slice = { port_buf, byteoffset(port_buf, mem_cstrlen(port_buf)) };
    u8_slice request = { req_buf, byteoffset(req_buf, mem_cstrlen(req_buf)) };

    void* res = https_request_sync(&alloc, host, port_slice, request);

    /* The function returns the initial 'begin' pointer on cleanup; ensure it did not advance/alter unexpectedly */
    TEST_ASSERT_EQUAL(t, (int)(res == begin), 1);

    sa_deinit(&alloc);
    mem_unmap(pointer, size);
}

void test_network_https_module(test_context* t) {
    REGISTER_TEST(t, "https_request_return_on_failed_resolve", test_https_request_returns_initial_pointer_on_failed_resolve);
}

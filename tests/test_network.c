#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>

#include "mem.h"
#include "stack_alloc.h"
#include "primitive.h"
#include "network/tcp/tcp_connection.h"
#include "network/https/https_request.h"
#include "test_network.h"
#include "print.h"

/*
    Tests for network layer:

    - test_tcp_connect_success
      Spins up a local TCP server on 127.0.0.1 with an ephemeral port.
      Calls tcp_connect(...) to that port and verifies the returned file descriptor
      is not file_invalid(). Cleans up sockets and joins server thread.

    - test_tcp_connect_failure
      Attempts to connect to a deliberately bogus hostname and expects tcp_connect
      to return file_invalid().

    - test_https_request_returns_initial_pointer_on_failed_resolve
      Verifies https_request_sync returns the initial allocator cursor when tcp_connect
      fails due to host resolution failure (no real network/SSL activity required).
*/

/* Thread argument for server thread */
typedef struct tcp_server_thread_arg {
    int listen_fd;
} tcp_server_thread_arg;

/* Server thread: accept one connection, optionally read a small amount, then close everything. */
static void* tcp_server_thread(void* arg) {
    tcp_server_thread_arg* a = (tcp_server_thread_arg*)arg;

    struct sockaddr_storage client_addr;
    socklen_t client_len = sizeof(client_addr);
    int client_fd = accept(a->listen_fd, (struct sockaddr*)&client_addr, &client_len);
    if (client_fd >= 0) {
        /* Try to read a bit (non-critical) then close */
        char buf[256];
        ssize_t r = read(client_fd, buf, sizeof(buf));
        unused(r);
        close(client_fd);
    }
    close(a->listen_fd);
    return NULL;
}

static void test_tcp_connect_success(test_context* t) {
    /* Create listening socket bound to 127.0.0.1:0 (ephemeral port) */
    int listen_fd = socket(AF_INET, SOCK_STREAM, 0);
    TEST_ASSERT_EQUAL(t, (int)(listen_fd >= 0), 1);

    int yes = 1;
    setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes));

    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK); /* 127.0.0.1 */
    addr.sin_port = 0; /* ephemeral */

    int b = bind(listen_fd, (struct sockaddr*)&addr, sizeof(addr));
    TEST_ASSERT_EQUAL(t, (int)(b == 0), 1);

    int l = listen(listen_fd, 1);
    TEST_ASSERT_EQUAL(t, (int)(l == 0), 1);

    /* Retrieve chosen port */
    struct sockaddr_in bound;
    socklen_t bound_len = sizeof(bound);
    int g = getsockname(listen_fd, (struct sockaddr*)&bound, &bound_len);
    TEST_ASSERT_EQUAL(t, (int)(g == 0), 1);
    uint16_t port = ntohs(bound.sin_port);

    /* Start server thread that will accept one connection */
    pthread_t thread;
    tcp_server_thread_arg arg = { .listen_fd = listen_fd };
    int pc = pthread_create(&thread, NULL, tcp_server_thread, &arg);
    TEST_ASSERT_EQUAL(t, (int)(pc == 0), 1);

    /* Prepare allocator for tcp_connect call */
    uptr size = 64 * 1024;
    void* pointer = mem_map(size);
    TEST_ASSERT_EQUAL(t, (int)(pointer != NULL), 1);
    stack_alloc alloc;
    sa_init(&alloc, pointer, byteoffset(pointer, size));

    /* Build u8_slice host and port */
    u8 host_buf[] = "127.0.0.1";
    u8 port_buf[16];
    snprintf((char*)port_buf, sizeof(port_buf), "%u", (unsigned)port);

    u8_slice host = { host_buf, byteoffset(host_buf, mem_cstrlen(host_buf)) };
    u8_slice port_slice = { port_buf, byteoffset(port_buf, mem_cstrlen(port_buf)) };

    /* Attempt to connect */
    file_t conn = tcp_connect(host, port_slice, &alloc);

    /* Expect success (not file_invalid) */
    TEST_ASSERT_EQUAL(t, (int)(conn != file_invalid()), 1);

    /* Close connection */
    tcp_close(conn);

    /* Join server thread */
    pthread_join(thread, NULL);

    /* Cleanup allocator */
    sa_deinit(&alloc);
    mem_unmap(pointer, size);
}

static void test_tcp_connect_failure(test_context* t) {
    /* Prepare allocator */
    uptr size = 32 * 1024;
    void* pointer = mem_map(size);
    TEST_ASSERT_EQUAL(t, (int)(pointer != NULL), 1);
    stack_alloc alloc;
    sa_init(&alloc, pointer, byteoffset(pointer, size));

    /* Deliberately bogus hostname that should not resolve */
    u8 host_buf[] = "no-such-hostname-for-unit-test.invalid";
    u8 port_buf[] = "12345";

    u8_slice host = { host_buf, byteoffset(host_buf, mem_cstrlen(host_buf)) };
    u8_slice port_slice = { port_buf, byteoffset(port_buf, mem_cstrlen(port_buf)) };

    file_t conn = tcp_connect(host, port_slice, &alloc);

    /* Expect failure (file_invalid) */
    TEST_ASSERT_EQUAL(t, (int)(conn == file_invalid()), 1);

    sa_deinit(&alloc);
    mem_unmap(pointer, size);
}

/* Test https_request_sync when tcp_connect fails (e.g. host cannot be resolved).
   The function should cleanly return the initial allocator cursor (no crash). */
static void test_https_request_returns_initial_pointer_on_failed_resolve(test_context* t) {
    uptr size = 64 * 1024;
    void* pointer = mem_map(size);
    TEST_ASSERT_EQUAL(t, (int)(pointer != NULL), 1);
    stack_alloc alloc;
    sa_init(&alloc, pointer, byteoffset(pointer, size));

    /* Save begin cursor */
    void* begin = alloc.cursor;

    u8 host_buf[] = "no-such-hostname-for-unit-test.invalid";
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

void test_network_module(test_context* t) {
    REGISTER_TEST(t, "tcp_connect_success", test_tcp_connect_success);
    REGISTER_TEST(t, "tcp_connect_failure", test_tcp_connect_failure);
    REGISTER_TEST(t, "https_request_return_on_failed_resolve", test_https_request_returns_initial_pointer_on_failed_resolve);
}

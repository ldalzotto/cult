

#include "mem.h"
#include "stack_alloc.h"
#include "primitive.h"
#include "network/tcp/tcp_connection.h"
#include "network/tcp/tcp_read_write.h"
#include "test_network_tcp.h"
#include "print.h"

/* Single-threaded non-blocking client/server test:
   - Create a listening server bound to 127.0.0.1:0 (ephemeral port)
   - Create a client and connect to the server (blocking connect for simplicity)
   - Accept the incoming connection on the server side
   - Set both the client and the accepted server socket to non-blocking mode
   - Use a simple main loop with select() to wait for readiness and transfer a
     message from client -> server using tcp_write_once / tcp_read_once.
   Memory for tcp handles and temporary IO buffers is provided only via
   stack_alloc as requested.
*/

// [TASK] This test should only use our own tcp module api
static void test_tcp_nonblocking_single_threaded(test_context* t) {
    /* Prepare allocator */
    uptr size = 64 * 1024;
    void* pointer = mem_map(size);
    TEST_ASSERT_TRUE(t, pointer != 0);
    stack_alloc alloc;
    sa_init(&alloc, pointer, byteoffset(pointer, size));

    /* Build host slice */
    string _host = STR("127.0.0.1");
    u8_slice host = {(void*)_host.begin, (void*)_host.end};

    string _port = STR("8000");
    u8_slice port = {(void*)_port.begin, (void*)_port.end};
    tcp* server = tcp_init_server(host, port, &alloc);
    TEST_ASSERT_TRUE(t, server != 0);
    TEST_ASSERT_NOT_EQUAL(t, (int)tcp_get_interal(server), (int)file_invalid());

    /* Create client handle (socket created, not yet connected) */
    tcp* client = tcp_init_client(host, port, &alloc);
    TEST_ASSERT_TRUE(t, client != 0);
    TEST_ASSERT_NOT_EQUAL(t, (int)tcp_get_interal(client), (int)file_invalid());

    /* Perform connect (blocking) to ensure the server has an incoming connection
       to accept. This simplifies the test while still allowing non-blocking IO
       for the subsequent read/write phase. */
    u8 connected = tcp_connect(client);
    TEST_ASSERT_TRUE(t, connected);

    /* Accept the incoming connection on the server (blocking accept is fine here). */
    tcp* server_peer = tcp_accept(server, &alloc);
    TEST_ASSERT_TRUE(t, server_peer != 0);
    TEST_ASSERT_NOT_EQUAL(t, (int)tcp_get_interal(server_peer), (int)file_invalid());

    /* Now set the client and the accepted server peer sockets to non-blocking. */
    u8 rc1 = tcp_set_nonblocking(client, 1);
    u8 rc2 = tcp_set_nonblocking(server_peer, 1);
    TEST_ASSERT_EQUAL(t, rc1, 1);
    TEST_ASSERT_EQUAL(t, rc2, 1);

    /* Message to send */
    const char msg[] = "hello from client";
    uptr msg_len = (uptr)mem_cstrlen((void*)msg);

    /* Main loop variables */
    uptr bytes_sent = 0;
    uptr bytes_received = 0;

    /* stack buffer to collect data after tcp_read_once returns an allocated slice */
    struct {
        void* begin;
        void* end;
    } recv_buffer;
    const uptr recv_buffer_size = 256;
    recv_buffer.begin = sa_alloc(&alloc, recv_buffer_size);
    recv_buffer.end = alloc.cursor;

    /* Loop until server has received the full message (client -> server) */
    for (i32 iter = 0; iter < 1000 && bytes_received < msg_len; ++iter) {

        /* If client writable, attempt to send remaining bytes */
        if (bytes_sent < msg_len && tcp_is_writable(client)) {
            u8_slice to_send = { (u8*)(msg + bytes_sent), byteoffset((msg + bytes_sent), msg_len) };
            tcp_w_result wres = tcp_write_once(client, to_send);
            if (wres.status == TCP_RW_OK && wres.bytes > 0) {
                bytes_sent += wres.bytes;
            } else if (wres.status == TCP_RW_ERR) {
                /* send would block or error; skip until next iteration */
                /* do nothing here */
            }
        }

        /* If server readable, attempt to read */
        if (tcp_is_readable(server_peer)) {
            /* We ask to read up to the remaining size */
            uptr want = (uptr)(msg_len - bytes_received);
            if (want > recv_buffer_size) want =  recv_buffer_size;

            u8_slice out;
            out.begin = alloc.cursor;
            tcp_r_result rres = tcp_read_once(server_peer, &alloc, want);
            out.end = alloc.cursor;
            if (rres.status == TCP_RW_OK && bytesize(out.begin, out.end) > 0) {
                /* Copy data out of the allocator-owned buffer into our local buffer
                   so we can sa_free the allocation and still inspect the data. */
                uptr got = bytesize(out.begin, out.end);
                TEST_ASSERT_TRUE(t, got <= recv_buffer_size);
                sa_copy(&alloc, out.begin, byteoffset(recv_buffer.begin, bytes_received), got);
                bytes_received += got;

                /* Free the allocation used by tcp_read_once */
                sa_free(&alloc, out.begin);
            } else if (rres.status == TCP_RW_EOF) {
                /* peer closed; nothing more will come */
                break;
            } else if (rres.status == TCP_RW_ERR) {
                /* read would block or error; skip */
            }
        }
    }

    /* Validate the message was fully received and matches */
    TEST_ASSERT_EQUAL(t, (int)bytes_received, (int)msg_len);
    if (bytes_received == msg_len) {
        /* Compare contents */
        u8 eq = sa_equals(&alloc, recv_buffer.begin, byteoffset(recv_buffer.begin, msg_len), msg, byteoffset(msg, msg_len));
        TEST_ASSERT_TRUE(t, eq);
    }

    sa_free(&alloc, recv_buffer.begin);

    /* Cleanup: close sockets and free tcp structs (allocated in stack_alloc) */
    tcp_close(server_peer);
    sa_free(&alloc, server_peer);

    tcp_close(client);
    sa_free(&alloc, client);

    tcp_close(server);
    sa_free(&alloc, server);

    /* Deinit allocator and unmap memory */
    sa_deinit(&alloc);
    mem_unmap(pointer, size);
}

void test_network_tcp_module(test_context* t) {
    REGISTER_TEST(t, "tcp_nonblocking_single_threaded", test_tcp_nonblocking_single_threaded);
}

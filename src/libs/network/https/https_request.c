#include "https_request.h"
#include "assert.h"
#include "network/tcp/tcp_connection.h"
#include "print.h"

#include <openssl/ssl.h>

static u8 ssl_write(SSL* ssl, void* begin, void* end) {
    void* cursor = begin;
    while (cursor < end) {
        i32 r = SSL_write(ssl, cursor, bytesize(cursor, end));
        if (r > 0) {
            cursor = byteoffset(cursor, r);
            debug_assert(cursor <= end);
            continue;
        }
        i32 err = SSL_get_error(ssl, r);
        if (err == SSL_ERROR_WANT_READ || err == SSL_ERROR_WANT_WRITE) continue;
        return 0;
    }
    return 1;
}

// Read the entire response (until connection closes)
static void* ssl_read(stack_alloc* alloc, SSL* ssl) {
    void* response_begin = alloc->cursor;
    while (1) {
        uptr size_remaining = bytesize(alloc->cursor, alloc->end);
        if (size_remaining > 8192) { size_remaining = 8192; }
        i32 r = SSL_read(ssl, alloc->cursor, size_remaining);
        if (r > 0) {
            alloc->cursor = byteoffset(alloc->cursor, r);
        } else if (r == 0) {
            // Clean EOF
            break;
        } else {
            // Error
            break;
        }
    }
    return response_begin;
}

void* https_request_sync(stack_alloc* alloc, u8_slice host, u8_slice port, u8_slice request) {
    void* begin = alloc->cursor;

    print_string(file_stdout(), STRING("Request:\n"));
    print_string(file_stdout(), (string){request.begin, request.end});
    print_string(file_stdout(), STRING("----"));

    struct {void* begin; void* end;} response;
    response.begin = alloc->cursor;
    response.end = alloc->cursor;

    // Initialize OpenSSL
    SSL_load_error_strings();
    OpenSSL_add_ssl_algorithms();

    const SSL_METHOD* method = TLS_client_method();
    if (!method) {
        goto deinit_method;
    }

    SSL_CTX* ctx = SSL_CTX_new(method);
    if (!ctx) {
        goto deinit_ssl_ctx;
    }
    SSL* ssl = SSL_new(ctx);
    if (!ssl) {
        goto deinit_ssl;
    }

    // Disable certifications for now
    SSL_CTX_set_verify(ctx, SSL_VERIFY_NONE, NULL);

    file_t tcp = tcp_connect(host, port, alloc);
    if (tcp == file_invalid()) {
        goto deinit_tcp_connect;
    }

    SSL_set_fd(ssl, tcp);

    void* host_null_terminated = sa_alloc(alloc, bytesize(host.begin, host.end) + 1);
    sa_copy(alloc, host.begin, host_null_terminated, bytesize(host.begin, host.end));
    *(u8*)byteoffset(host_null_terminated, bytesize(host.begin, host.end)) = '\0';
    SSL_set_tlsext_host_name(ssl, host_null_terminated);
    sa_free(alloc, host_null_terminated);

    if (SSL_connect(ssl) != 1) {
        goto deinit_ssl_connect;
    }

    // Send request
    if (!ssl_write(ssl, request.begin, request.end)) {
        int debug = 10;
        unused(debug);
    }

    // Read the response
    response.begin = ssl_read(alloc, ssl);
    response.end = alloc->cursor;

    // print_format(file_stdout(), STRING("%s\n"), (string){response.begin, response.end});

    SSL_shutdown(ssl);
deinit_ssl_connect:
    tcp_close(tcp);
deinit_tcp_connect:
    SSL_free(ssl);
deinit_ssl:
    SSL_CTX_free(ctx);
deinit_ssl_ctx:
deinit_method:

    sa_move_tail(alloc, response.begin, begin);

    return begin;
}



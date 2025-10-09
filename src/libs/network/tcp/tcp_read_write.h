
#ifndef TCP_READ_WRITE_H
#define TCP_READ_WRITE_H

#include "tcp_connection.h"

/* Result status for read/write operations */
typedef enum {
    TCP_RW_OK = 0,  /* Operation succeeded */
    TCP_RW_EOF = 1, /* Orderly EOF (peer closed connection) */
    TCP_RW_ERR = 2  /* Error occurred; errno preserved by syscall */
} tcp_rw_status;

typedef struct {
    tcp_rw_status status; /* outcome of the read operation */
} tcp_r_result;

typedef struct {
    tcp_rw_status status; /* outcome of the write operation */
    uptr bytes;           /* number of bytes written (meaningful for write ops) */
} tcp_w_result;

/*
 * Read at most max_len bytes from the tcp connection in a single recv() call.
 *
 * - Buffer is taken from alloc->cursor; on success alloc->cursor is advanced
 *   by the number of bytes received.
 * - Allocation is performed before recv(); it is not rolled back if recv fails.
 * - Returns:
 *     TCP_RW_OK  : bytes were read and alloc->cursor advanced
 *     TCP_RW_EOF : orderly EOF (peer closed connection)
 *     TCP_RW_ERR : error occurred (errno set by recv)
 *
 * Note: This performs a single recv() attempt. Caller must retry as needed.
 */
tcp_r_result tcp_read_once(tcp* connection, stack_alloc* alloc, uptr max_len);

/*
 * Write data to the tcp connection in a single send() call.
 *
 * - data describes the buffer to send (data.begin .. data.end).
 * - Returns:
 *     TCP_RW_OK  : res.bytes contains number of bytes written (may be 0 or < len)
 *     TCP_RW_ERR : error occurred (errno set by send)
 *
 * Note: This performs a single send() attempt. Caller must retry to send
 * remaining data if necessary.
 */
tcp_w_result tcp_write_once(tcp* connection, u8_slice data);

#endif /* TCP_READ_WRITE_H */

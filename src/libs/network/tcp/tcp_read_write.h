#ifndef TCP_READ_WRITE_H
#define TCP_READ_WRITE_H

#include "tcp_connection.h"

/* Result status for read/write operations */
typedef enum {
    TCP_RW_OK = 0,  /* Operation succeeded (may be 0 bytes for benign cases) */
    TCP_RW_EOF = 1, /* Orderly EOF (peer closed connection) */
    TCP_RW_ERR = 2  /* Error occurred; errno is provided in .err */
} tcp_rw_status;


typedef struct {
    tcp_rw_status status; /* outcome of the operation */
} tcp_r_result;

typedef struct {
    tcp_rw_status status; /* outcome of the operation */
    uptr bytes;           /* number of bytes written (meaningful for write ops) */
} tcp_w_result;

/*
 * Read at most max_len bytes from the tcp connection in a single attempt.
 * The function will allocate a buffer from the provided stack_alloc and
 * place the received data into that allocation. The u8_slice 'out' will
 * be set to point into the stack allocation (out->begin .. out->end).
 *
 * Returns:
 *  status == TCP_RW_OK  : data has been placed into the allocation; number
 *                         of bytes read is implied by the alloc cursor
 *                         (do not inspect .bytes for read operations).
 *  status == TCP_RW_EOF : orderly EOF (peer closed connection)
 *  status == TCP_RW_ERR : error, err contains errno (including EAGAIN/EWOULDBLOCK)
 *
 * NOTE: This function performs a single recv() attempt. Callers are
 * responsible for retrying as needed. The allocation is performed before
 * calling recv(); if recv fails the allocation is not rolled back.
 */
tcp_r_result tcp_read_once(tcp* connection, stack_alloc* alloc, uptr max_len);

/*
 * Write data to the tcp connection in a single attempt.
 *
 * Parameters:
 *  - connection: tcp handle
 *  - data: u8_slice describing the buffer to send (begin..end)
 *
 * Returns:
 *  status == TCP_RW_OK  : bytes indicates number of bytes written (may be less than data length or 0)
 *  status == TCP_RW_ERR : error, err contains errno (including EAGAIN/EWOULDBLOCK)
 *
 * NOTE: This performs a single send() attempt. Caller must retry to send
 * remaining data if necessary.
 */
tcp_w_result tcp_write_once(tcp* connection, u8_slice data);

#endif /* TCP_READ_WRITE_H */

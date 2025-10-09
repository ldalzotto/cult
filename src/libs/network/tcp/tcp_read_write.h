#ifndef TCP_READ_WRITE_H
#define TCP_READ_WRITE_H

#include "tcp_connection.h"

typedef enum {
    TCP_RW_OK = 0,
    TCP_RW_EOF = 1,
    TCP_RW_ERR = 2
} tcp_rw_status;

typedef struct {
    tcp_rw_status status;
} tcp_r_result;

typedef struct {
    tcp_rw_status status;
    uptr bytes;
} tcp_w_result;

/* Read at most max_len bytes once. Buffer is taken from alloc->cursor. */
tcp_r_result tcp_read_once(tcp* connection, stack_alloc* alloc, uptr max_len);

/* Write data once. res.bytes holds bytes written on success. */
tcp_w_result tcp_write_once(tcp* connection, u8_slice data);

#endif /* TCP_READ_WRITE_H */

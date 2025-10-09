
#include "tcp_read_write.h"
#include "tcp_connection_type.h"

#include <sys/socket.h>
#include <unistd.h>
#include <string.h>

tcp_r_result tcp_read_once(tcp* connection, stack_alloc* alloc, uptr max_len) {
    tcp_r_result res;
    res.status = TCP_RW_ERR;

    if (connection->fd == file_invalid()) return res;
    if (max_len == 0) {
        /* Nothing to read */
        res.status = TCP_RW_OK;
        return res;
    }

    /* Allocate buffer directly from the provided stack allocator. */
    void* buf = alloc->cursor;

    ssize_t rc = recv((int)connection->fd, buf, max_len, 0);
    if (rc > 0) {
        alloc->cursor = byteoffset(alloc->cursor, rc);
        res.status = TCP_RW_OK;
        return res;
    } else if (rc == 0) {
        /* orderly shutdown by peer */
        res.status = TCP_RW_EOF;
        return res;
    } else {
        /* rc == -1 : error, errno preserved by recv */
        res.status = TCP_RW_ERR;
        return res;
    }
}

tcp_w_result tcp_write_once(tcp* connection, u8_slice data) {
    tcp_w_result res;
    res.status = TCP_RW_ERR;
    res.bytes = 0;

    if (connection->fd == file_invalid()) return res;

    uptr len = bytesize(data.begin, data.end);
    if (len == 0) {
        res.status = TCP_RW_OK;
        res.bytes = 0;
        return res;
    }

    ssize_t rc = send((int)connection->fd, data.begin, (size_t)len, 0);
    if (rc < 0) {
        res.status = TCP_RW_ERR;
        res.bytes = 0;
    } else {
        res.status = TCP_RW_OK;
        res.bytes = (uptr)rc;
    }
    return res;
}

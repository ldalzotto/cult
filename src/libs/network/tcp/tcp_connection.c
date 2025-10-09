#include "tcp_connection.h"
#include "mem.h"
#include "print.h"
#include "assert.h"

#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>


struct tcp {
    file_t fd;
    struct addrinfo* res;
    struct addrinfo* chosen;
};

tcp* tcp_init(u8_slice host, u8_slice port, stack_alloc* alloc) {
    struct addrinfo hints;
    for  (u8* c = (u8*)&hints; c < (u8*)(&hints + 1); ++c) {*c = 0;}
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_family = AF_UNSPEC;
    hints.ai_protocol = IPPROTO_TCP;
    
    u8* host_null_terminated = sa_alloc(alloc, bytesize(host.begin, host.end) + 1);
    sa_copy(alloc, host.begin, host_null_terminated, bytesize(host.begin, host.end));
    host_null_terminated[bytesize(host.begin, host.end)] = '\0';

    u8* port_null_terminated = sa_alloc(alloc, bytesize(port.begin, port.end) + 1);
    sa_copy(alloc, port.begin, port_null_terminated, bytesize(port.begin, port.end));
    port_null_terminated[bytesize(port.begin, port.end)] = '\0';

    struct addrinfo* res = NULL;
    int gai = getaddrinfo((const char*)host_null_terminated, (const char*)port_null_terminated, &hints, &res);
    sa_free(alloc, port_null_terminated);
    sa_free(alloc, host_null_terminated);

    if (gai != 0) {
        const char* error = gai_strerror(gai);
        print_format(file_stdout(), STRING("%s\n"), (string){error, byteoffset(error, mem_cstrlen((void*)error))});
        return NULL;
    } 

    tcp* connection = sa_alloc(alloc, sizeof(tcp));
    for (u8* c = (u8*)connection; c < (u8*)(connection + 1); ++c) {*c = 0;}

    connection->res = res;
    connection->chosen = NULL;
    connection->fd = file_invalid();

    /* Only create the socket here (from the old tcp_connect) -- do not call connect(). */
    for (struct addrinfo* rp = res; rp; rp = rp->ai_next) {
        int fd = (int)socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
        if (fd < 0) continue;
        connection->fd = (file_t)fd;
        connection->chosen = rp;
        break;
    }

    if (connection->fd == file_invalid()) {
        /* no socket could be created for any addrinfo */
        freeaddrinfo(res);
        sa_free(alloc, connection);
        return NULL;
    }

    return connection;
}

u8 tcp_connect(tcp* connection) {
    debug_assert(connection->fd != file_invalid());
    debug_assert(connection->chosen != NULL);

    int rc = connect((int)connection->fd, connection->chosen->ai_addr, connection->chosen->ai_addrlen);
    return rc == 0;
}

file_t tcp_get_interal(tcp* connection) {
    return connection->fd;
}

void tcp_close(tcp* connection) {
    if (!connection) return;

    if (connection->fd != file_invalid()) {
        close((int)connection->fd);
        connection->fd = file_invalid();
    }

    if (connection->res) {
        freeaddrinfo(connection->res);
        connection->res = NULL;
        connection->chosen = NULL;
    }
}

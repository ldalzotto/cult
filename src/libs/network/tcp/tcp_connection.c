
#include "tcp_connection.h"
#include "tcp_connection_type.h"
#include "mem.h"
#include "print.h"
#include "assert.h"

#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <string.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <errno.h>


static size_t slice_len(u8_slice s) {
    return bytesize(s.begin, s.end);
}

static char* make_cstr_from_slice(stack_alloc* alloc, u8_slice s) {
    size_t len = slice_len(s);
    char* out = (char*)sa_alloc(alloc, len + 1);
    sa_copy(alloc, s.begin, (u8*)out, len);
    out[len] = '\0';
    return out;
}

tcp* tcp_init_client(u8_slice host, u8_slice port, stack_alloc* alloc) {
    struct addrinfo hints;
    for (u8* c = (u8*)&hints; c < (u8*)(&hints + 1); ++c) {*c = 0;}
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_family = AF_UNSPEC;
    hints.ai_protocol = IPPROTO_TCP;
    /* No AI_PASSIVE for client. */

    char* host_c = make_cstr_from_slice(alloc, host);
    char* port_c = make_cstr_from_slice(alloc, port);

    struct addrinfo* res = NULL;
    int gai = getaddrinfo((const char*)host_c, (const char*)port_c, &hints, &res);
    sa_free(alloc, (u8*)port_c);
    sa_free(alloc, (u8*)host_c);

    tcp* connection = sa_alloc(alloc, sizeof(tcp));
    connection->res = res;
    connection->chosen = NULL;
    connection->fd = file_invalid();

    if (gai != 0) {
        const char* error = gai_strerror(gai);
        print_format(file_stdout(), STRING("%s\n"), (string){error, byteoffset(error, mem_cstrlen((void*)error))});
        return connection;
    }

    /* Create a socket for the first address that works (do not connect yet). */
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
        connection->res = NULL;
        return connection;
    }

    return connection;
}

tcp* tcp_init_server(u8_slice host, u8_slice port, stack_alloc* alloc) {
    struct addrinfo hints;
    for (u8* c = (u8*)&hints; c < (u8*)(&hints + 1); ++c) {*c = 0;}
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_family = AF_UNSPEC;
    hints.ai_protocol = IPPROTO_TCP;
    hints.ai_flags = AI_PASSIVE; /* suitable for bind() */

    char* host_c = NULL;
    size_t hlen = slice_len(host);
    if (hlen > 0) {
        host_c = make_cstr_from_slice(alloc, host);
    }
    char* port_c = make_cstr_from_slice(alloc, port);

    struct addrinfo* res = NULL;
    int gai = getaddrinfo((const char*)host_c, (const char*)port_c, &hints, &res);
    sa_free(alloc, (u8*)port_c);
    if (host_c) sa_free(alloc, (u8*)host_c);
    

    if (gai != 0) {
        const char* error = gai_strerror(gai);
        print_format(file_stdout(), STRING("%s\n"), (string){error, byteoffset(error, mem_cstrlen((void*)error))});
        return NULL;
    }

    tcp* connection = sa_alloc(alloc, sizeof(tcp));

    connection->res = res;
    connection->chosen = NULL;
    connection->fd = file_invalid();

    /* Try to create, bind and listen for each addrinfo until success. */
    for (struct addrinfo* rp = res; rp; rp = rp->ai_next) {
        int fd = (int)socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
        if (fd < 0) continue;

        int opt = 1;
        setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

        if (bind(fd, rp->ai_addr, rp->ai_addrlen) != 0) {
            close(fd);
            continue;
        }

        if (listen(fd, SOMAXCONN) != 0) {
            close(fd);
            continue;
        }

        connection->fd = (file_t)fd;
        connection->chosen = rp;
        break;
    }

    if (connection->fd == file_invalid()) {
        /* failed to bind/listen on any addrinfo */
        freeaddrinfo(res);
        connection->res = NULL;
        return connection;
    }

    return connection;
}

u8 tcp_connect(tcp* connection) {
    debug_assert(connection != NULL);
    debug_assert(connection->fd != file_invalid());
    debug_assert(connection->chosen != NULL);

    int rc = connect((int)connection->fd, connection->chosen->ai_addr, connection->chosen->ai_addrlen);
    return rc == 0;
}

/* Accept an incoming connection on a listening server socket and return a new
   tcp handle representing the accepted client socket. The returned tcp has
   res and chosen set to NULL because accept() does not provide addrinfo. */
tcp* tcp_accept(tcp* server, stack_alloc* alloc) {
    debug_assert(server->fd != file_invalid());

    struct sockaddr_storage addr;
    socklen_t addrlen = sizeof(addr);
    int client_fd = accept((int)server->fd, (struct sockaddr*)&addr, &addrlen);
    if (client_fd < 0) {
        return 0;
    }

    tcp* connection = sa_alloc(alloc, sizeof(tcp));
    connection->fd = (file_t)client_fd;
    connection->res = NULL;
    connection->chosen = NULL;
    return connection;
}


/* Set or clear non-blocking mode on the socket.
   nonblocking != 0 -> set O_NONBLOCK
   nonblocking == 0 -> clear O_NONBLOCK
   Returns 0 on success, -1 on error (check errno). */
int tcp_set_nonblocking(tcp* connection, u8 nonblocking) {
    debug_assert(connection->fd != file_invalid());

    int fd = (int)connection->fd;
    int flags = fcntl(fd, F_GETFL, 0);
    if (flags == -1) return -1;

    if (nonblocking) {
        if (flags & O_NONBLOCK) return 0; /* already non-blocking */
        if (fcntl(fd, F_SETFL, flags | O_NONBLOCK) == -1) return -1;
    } else {
        if (!(flags & O_NONBLOCK)) return 0; /* already blocking */
        if (fcntl(fd, F_SETFL, flags & ~O_NONBLOCK) == -1) return -1;
    }
    return 0;
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

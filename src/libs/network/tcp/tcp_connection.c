#include "tcp_connection.h"
#include "mem.h"
#include "print.h"

#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>

file_t tcp_connect(u8_slice host, u8_slice port, stack_alloc* alloc) {
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
        return file_invalid();
    } 

    int fd = file_invalid();
    for (struct addrinfo* rp = res; rp; rp = rp->ai_next) {
        fd = (int)socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
        if (fd < 0) continue;
        if (connect(fd, rp->ai_addr, rp->ai_addrlen) == 0) {
            break; // success
        }
        close(fd);
        fd = file_invalid();
    }

    freeaddrinfo(res);
    return fd;
}

void tcp_close(file_t connection) {
    close(connection);
}


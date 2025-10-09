#ifndef TCP_CONNECTION_H
#define TCP_CONNECTION_H

#include "primitive.h"
#include "stack_alloc.h"
#include "file.h"

/* opaque tcp handle */
typedef struct tcp tcp;

/* Create a client tcp handle (resolve host/port, create socket). */
tcp* tcp_init_client(u8_slice host, u8_slice port, stack_alloc* alloc);

/* Create a server tcp handle (resolve, bind, listen). */
tcp* tcp_init_server(u8_slice host, u8_slice port, stack_alloc* alloc);

/* Connect a client socket (returns 1 on success). For server returns 1. */
u8 tcp_connect(tcp* connection);

/* Accept an incoming connection on a listening server socket. */
tcp* tcp_accept(tcp* server, stack_alloc* alloc);

/* Set or clear non-blocking mode (returns 1 on success). */
u8 tcp_set_nonblocking(tcp* connection, u8 nonblocking);

u8 tcp_is_writable(tcp* connection);
u8 tcp_is_readable(tcp* connection);

/* Get the internal file/socket descriptor. */
file_t tcp_get_interal(tcp* connection);

/* Close socket and free addrinfo; does not free tcp struct. */
void tcp_close(tcp* connection);

#endif /* TCP_CONNECTION_H */

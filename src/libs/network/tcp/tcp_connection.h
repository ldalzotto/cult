
#ifndef TCP_CONNECTION_H
#define TCP_CONNECTION_H

#include "primitive.h"
#include "stack_alloc.h"
#include "file.h"

/* opaque tcp handle */
typedef struct tcp tcp;

/* Initialize a tcp handle for a client. This will resolve the host/port
   and create a socket (but will NOT call connect()). */
tcp* tcp_init_client(u8_slice host, u8_slice port, stack_alloc* alloc);

/* Initialize a tcp handle for a server. This will resolve the host/port,
   create a socket, bind and listen on it. */
tcp* tcp_init_server(u8_slice host, u8_slice port, stack_alloc* alloc);

/* For client connections, attempt to connect the created socket to the
   chosen address (returns 1 on success). For server sockets this returns 1
   because the socket is already bound and listening. */
u8 tcp_connect(tcp* connection);

/* Wait for an incoming connection on a server tcp handle and return a new
   tcp handle for the accepted client connection. */
tcp* tcp_accept(tcp* server, stack_alloc* alloc);

/* Set or clear non-blocking mode on the socket.
   nonblocking != 0 -> set O_NONBLOCK
   nonblocking == 0 -> clear O_NONBLOCK
   Returns 0 on success, -1 on error (check errno). */
int tcp_set_nonblocking(tcp* connection, u8 nonblocking);

/* Get the internal file/socket descriptor. */
file_t tcp_get_interal(tcp* connection);

/* Close socket and free addrinfo; does not free the tcp struct itself. */
void tcp_close(tcp* connection);

#endif /*TCP_CONNECTION_H*/

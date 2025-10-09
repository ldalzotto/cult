#ifndef TCP_CONNECTION_H
#define TCP_CONNECTION_H

#include "primitive.h"
#include "stack_alloc.h"
#include "file.h"

typedef struct tcp tcp;
tcp* tcp_init(u8_slice host, u8_slice port, stack_alloc* alloc);
u8 tcp_connect(tcp* connection);
file_t tcp_get_interal(tcp* connection);
void tcp_close(tcp* connection);

#endif /*TCP_CONNECTION_H*/

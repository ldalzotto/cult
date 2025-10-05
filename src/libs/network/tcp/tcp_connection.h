#ifndef TCP_CONNECTION_H
#define TCP_CONNECTION_H

#include "primitive.h"
#include "stack_alloc.h"
#include "file.h"

file_t tcp_connect(u8_slice host, u8_slice port, stack_alloc* alloc);
void tcp_close(file_t connection);

#endif /*TCP_CONNECTION_H*/

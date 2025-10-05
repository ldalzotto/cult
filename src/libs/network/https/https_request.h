#ifndef HTTPS_REQUEST_H
#define HTTPS_REQUEST_H

#include "stack_alloc.h"

void* https_request_sync(stack_alloc* alloc, u8_slice host, u8_slice port, u8_slice request);

#endif

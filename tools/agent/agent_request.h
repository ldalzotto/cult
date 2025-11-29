#ifndef AGENT_REQUEST_H
#define AGENT_REQUEST_H

#include "stack_alloc.h"
#include "litteral.h"

u8* agent_request(u8_slice user_content, string api_key, string model, string prompt_id, stack_alloc* alloc);

#endif /*AGENT_REQUEST_H*/

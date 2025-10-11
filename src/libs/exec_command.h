#ifndef EXEC_COMMAND_H
#define EXEC_COMMAND_H

#include "litteral.h"
#include "stack_alloc.h"

typedef struct {
    void* output;
    u8 success;
} exec_command_result;

/*
    Execute a command. Returns a pointer in alloc that is the start of the result.
*/
exec_command_result exec_command(const string cmd, stack_alloc* alloc);

#endif /* EXEC_COMMAND_H */

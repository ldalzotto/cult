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

typedef struct exec_command_session exec_command_session;
exec_command_session* open_persistent_shell(stack_alloc* alloc);
exec_command_result command_session_exec_command(exec_command_session* session, const string cmd, stack_alloc* alloc);
void close_persistent_shell(exec_command_session* session);

#endif /* EXEC_COMMAND_H */

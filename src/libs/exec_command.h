#ifndef EXEC_COMMAND_H
#define EXEC_COMMAND_H

#include "litteral.h"
#include "stack_alloc.h"

/*
    Execute a command. Returns a pointer in alloc that is the start of the result.
*/
void* exec_command(const string cmd, stack_alloc* alloc);

#endif /* EXEC_COMMAND_H */

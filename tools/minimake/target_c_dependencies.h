#ifndef MINIMAKE_TARGET_C_DEPENDENCIES_H
#define MINIMAKE_TARGET_C_DEPENDENCIES_H

#include "litteral.h"
#include "stack_alloc.h"
#include "exec_command.h"

string* extract_c_dependencies(string name, string template, exec_command_session* session, stack_alloc* alloc);

#endif // MINIMAKE_TARGET_C_DEPENDENCIES_H

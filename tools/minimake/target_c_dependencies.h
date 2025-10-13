#ifndef MINIMAKE_TARGET_C_DEPENDENCIES_H
#define MINIMAKE_TARGET_C_DEPENDENCIES_H

#include "litteral.h"
#include "stack_alloc.h"

string* extract_c_dependencies(string name, string template, stack_alloc* alloc);

#endif // MINIMAKE_TARGET_C_DEPENDENCIES_H

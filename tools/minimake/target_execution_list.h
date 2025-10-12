#ifndef MINIMAKE_TARGET_EXECUTION_LIST_H
#define MINIMAKE_TARGET_EXECUTION_LIST_H

#include "target.h"
#include "stack_alloc.h"

target** target_execution_list(target* targets_begin, target* targets_end, target* main, stack_alloc* alloc);

#endif // MINIMAKE_TARGET_EXECUTION_LIST_H

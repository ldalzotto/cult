#ifndef MINIMAKE_TARGET_H
#define MINIMAKE_TARGET_H

#include "stack_alloc.h"
#include "litteral.h"

typedef struct target target;
typedef u8(*target_build_cb)(target* t, string cache_dir, stack_alloc* alloc);

struct target {
    string name;
    target_build_cb build;
    string* deps;
    void* end;
};

#endif // MINIMAKE_TARGET_H

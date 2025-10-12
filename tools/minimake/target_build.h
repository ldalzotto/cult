#ifndef MINIMAKE_TARGET_BUILD_H
#define MINIMAKE_TARGET_BUILD_H

#include "target.h"

u8 target_build_object(target* t, string cache_dir, stack_alloc* alloc);
u8 target_build_executable(target* t, string cache_dir, stack_alloc* alloc);
u8 target_build(target* targets_begin, target* targets_end, target* target_to_build, string cache_dir, stack_alloc* alloc);

#endif // MINIMAKE_TARGET_BUILD_H

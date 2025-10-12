#ifndef MINIMAKE_TARGET_TIMESTAMP_H
#define MINIMAKE_TARGET_TIMESTAMP_H

#include "primitive.h"
#include "stack_alloc.h"
#include "target.h"

void timestamp_write(const string name, const string cache_dir, uptr ts, stack_alloc* alloc);
uptr timestamp_read(const string name, const string cache_dir, stack_alloc* alloc);
void target_update_timestamp(target* t, string cache_dir, stack_alloc* alloc);

#endif // MINIMAKE_TARGET_TIMESTAMP_H

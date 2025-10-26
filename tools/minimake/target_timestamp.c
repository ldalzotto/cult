#include "primitive.h"
#include "stack_alloc.h"
#include "file.h"
#include "target_timestamp.h"

void timestamp_write(const string name, const string cache_dir, uptr ts, stack_alloc* alloc) {
    string name_path;
    name_path.begin = sa_alloc_copy(alloc, cache_dir.begin, cache_dir.end);
    sa_alloc_copy(alloc, name.begin, name.end);
    name_path.end = alloc->cursor;

    directory_create_for_file(alloc, name_path.begin, name_path.end, DIR_MODE_PUBLIC);
    file_t file = file_open(alloc, name_path.begin, name_path.end, FILE_MODE_WRITE);

    file_write(file, &ts, (&ts) + 1);
    file_close(file);

    sa_free(alloc, (void*)name_path.begin);
}

uptr timestamp_read(const string name, const string cache_dir, stack_alloc* alloc) {
    string name_path;
    name_path.begin = sa_alloc_copy(alloc, cache_dir.begin, cache_dir.end);
    sa_alloc_copy(alloc, name.begin, name.end);
    name_path.end = alloc->cursor;

    uptr ts = 0;
    file_t file = file_open(alloc, name_path.begin, name_path.end, FILE_MODE_READ);
    if (file != file_invalid()) {
        ts = file_modification_time(file);
        file_close(file);
    }

    sa_free(alloc, (void*)name_path.begin);

    return ts;
}

void target_update_timestamp(target* t, string cache_dir, stack_alloc* alloc) {
    uptr ts_to_write = 0;
    for (string* d = t->deps; (void*)d < t->end;) {
        uptr ts = timestamp_read(*d, cache_dir, alloc);
        if (ts > ts_to_write) {ts_to_write = ts;}
        d = (void*)d->end;
    }
    timestamp_write(t->name, cache_dir, ts_to_write, alloc);
}

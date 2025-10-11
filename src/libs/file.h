#ifndef FILE_H
#define FILE_H

#include "primitive.h"
#include "stack_alloc.h"

// File handle
typedef i32 file_t;
file_t file_invalid(void);

// File open modes
typedef enum {
    FILE_MODE_READ,
    FILE_MODE_WRITE,
    FILE_MODE_READ_WRITE
} file_mode_t;

// File operations
file_t file_open(stack_alloc* alloc, const u8* path_begin, const u8* path_end, file_mode_t mode);
void file_close(file_t file);

// Console file handles
file_t file_stdout(void);
file_t file_stderr(void);

// Read operations
uptr file_read_all(file_t file, void** buffer, stack_alloc* alloc);

// Write operations
uptr file_write(file_t file, const void* begin, const void* end);

// Utility functions
uptr file_size(file_t file);

// Returns the modification time (milliseconds since epoch) for an open file handle.
uptr file_modification_time(file_t file);

typedef enum {
    DIR_MODE_DEFAULT, // typical user-accessible
    DIR_MODE_PRIVATE, // only owner
    DIR_MODE_PUBLIC   // world-accessible
} dir_mode_t;

i32 directory_create(stack_alloc* alloc, const u8* path_begin, const u8* path_end, dir_mode_t mode);
i32 directory_create_for_file(stack_alloc *alloc, const u8 *path_begin, const u8 *path_end, dir_mode_t mode);
i32 directory_remove(stack_alloc* alloc, const u8* path_begin, const u8* path_end);

#endif /* FILE_H */

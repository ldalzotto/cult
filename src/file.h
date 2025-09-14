#ifndef FILE_H
#define FILE_H

#include "primitive.h"
#include "stack_alloc.h"

// File handle
typedef void* file_t;

// File open modes
typedef enum {
    FILE_MODE_READ,
    FILE_MODE_WRITE,
    FILE_MODE_READ_WRITE
} file_mode_t;

// File operations
file_t file_open(const char* path, file_mode_t mode);
void file_close(file_t file);

// Read operations
uptr file_read_all(file_t file, void** buffer, stack_alloc* alloc);

// Write operations
uptr file_write(file_t file, const void* buffer, uptr size);

// Utility functions
uptr file_size(file_t file);

#endif /* FILE_H */

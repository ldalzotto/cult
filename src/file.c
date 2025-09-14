#include "file.h"
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>

// Open a file with the specified mode
file_t file_open(const char* path, file_mode_t mode) {
    int flags = 0;
    switch (mode) {
        case FILE_MODE_READ:
            flags = O_RDONLY;
            break;
        case FILE_MODE_WRITE:
            flags = O_WRONLY | O_CREAT | O_TRUNC;
            break;
        case FILE_MODE_READ_WRITE:
            flags = O_RDWR | O_CREAT;
            break;
        default:
            return NULL;
    }

    int fd = open(path, flags, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
    if (fd == -1) {
        return NULL;
    }
    return (file_t)(uptr)fd;
}

// Close a file
void file_close(file_t file) {
    if (file) {
        close((int)(uptr)file);
    }
}

// Read entire file into allocated buffer using stack allocator
uptr file_read_all(file_t file, void** buffer, stack_alloc* alloc) {
    if (!file || !buffer || !alloc) {
        return 0;
    }

    int fd = (int)(uptr)file;

    // Get file size
    off_t file_size = lseek(fd, 0, SEEK_END);
    lseek(fd, 0, SEEK_SET);

    if (file_size <= 0) {
        return 0;
    }

    // Allocate buffer
    *buffer = sa_alloc(alloc, (uptr)file_size);
    if (!*buffer) {
        return 0;
    }

    // Read entire file
    ssize_t bytes_read = read(fd, *buffer, (size_t)file_size);
    if (bytes_read == -1) {
        return 0;
    }
    return (uptr)bytes_read;
}

// Write data to file
uptr file_write(file_t file, const void* buffer, uptr size) {
    if (!file || !buffer) {
        return 0;
    }

    int fd = (int)(uptr)file;
    ssize_t bytes_written = write(fd, buffer, (size_t)size);
    if (bytes_written == -1) {
        return 0;
    }
    return (uptr)bytes_written;
}

// Get file size
uptr file_size(file_t file) {
    if (!file) {
        return 0;
    }

    int fd = (int)(uptr)file;

    // Save current position
    off_t current_pos = lseek(fd, 0, SEEK_CUR);

    // Get file size
    off_t size = lseek(fd, 0, SEEK_END);

    // Restore position
    lseek(fd, current_pos, SEEK_SET);

    if (size == -1) {
        return 0;
    }
    return (uptr)size;
}

#include "file.h"
#include "./assert.h"
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <errno.h>
#include <string.h>
#include <dirent.h>
#include <stdio.h>

file_t file_invalid(void) {
    return -1;
}

// Open a file with the specified mode using stack_alloc
file_t file_open(stack_alloc* alloc, const u8* path_begin, const u8* path_end, file_mode_t mode) {
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
            return file_invalid();
    }

    // Calculate path length
    uptr path_len = bytesize(path_begin, path_end);

    // Allocate path buffer from stack allocator
    u8* path = sa_alloc(alloc, path_len + 1);
    memcpy(path, path_begin, path_len);
    path[path_len] = '\0';

    int fd = open((void*)path, flags, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);

    if (fd == file_invalid()) {
        // Optional: rollback allocation if open fails
        sa_free(alloc, path);
        return fd;
    }

    // Free path buffer after use since open() doesn't need it after call
    sa_free(alloc, path);

    return fd;
}

// Close a file
void file_close(file_t file) {
    debug_assert(file != file_stderr());
    debug_assert(file != file_stdout());
    if (file) {
        close(file);
    }
}

// Read entire file into allocated buffer using stack allocator
uptr file_read_all(file_t file, void** buffer, stack_alloc* alloc) {
    if (!file || !buffer || !alloc) {
        return 0;
    }

    // Get file size
    off_t file_size = lseek(file, 0, SEEK_END);
    lseek(file, 0, SEEK_SET);

    if (file_size <= 0) {
        return 0;
    }

    // Allocate buffer
    *buffer = sa_alloc(alloc, (uptr)file_size);
    if (!*buffer) {
        return 0;
    }

    // Read entire file
    ssize_t bytes_read = read(file, *buffer, (size_t)file_size);
    if (bytes_read == file_invalid()) {
        return 0;
    }
    debug_assert(file_size == bytes_read);
    return (uptr)bytes_read;
}

// Write data to file
uptr file_write(file_t file, const void* buffer, uptr size) {
    ssize_t bytes_written = write(file, buffer, (size_t)size);
    if (bytes_written == file_invalid()) {
        return 0;
    }
    return (uptr)bytes_written;
}

// Get file size
uptr file_size(file_t file) {
    // Save current position
    off_t current_pos = lseek(file, 0, SEEK_CUR);

    // Get file size
    off_t size = lseek(file, 0, SEEK_END);

    // Restore position
    lseek(file, current_pos, SEEK_SET);

    if (size == -1) {
        return 0;
    }
    return (uptr)size;
}

// Create directory recursively (like mkdir -p)
i32 directory_create(stack_alloc* alloc, const u8* path_begin, const u8* path_end, dir_mode_t mode) {

    // Allocate contiguous buffer for path
    uptr path_len = bytesize(path_begin, path_end);
    u8* path_buf = sa_alloc(alloc, path_len + 1);
    memcpy(path_buf, path_begin, path_len);
    path_buf[path_len] = '\0';

    // Iterate through path segments
    u8* segment_start = path_buf;
    u8* cursor = path_buf;

    while ((void*)cursor <= byteoffset(path_buf, path_len)) {
        if (*cursor == '/' || *cursor == '\0') {
            *cursor = '\0'; // terminate segment

            // Map dir_mode_t to platform permission inline
            u32 perms;
            switch (mode) {
                case DIR_MODE_DEFAULT: perms = 0755; break;
                case DIR_MODE_PRIVATE: perms = 0700; break;
                case DIR_MODE_PUBLIC:  perms = 0777; break;
                default:               perms = 0755; break;
            }

            // Create directory if it doesn't exist
            if (access((void*)segment_start, F_OK) != 0) {
                if (mkdir((void*)segment_start, perms) != 0 && errno != EEXIST) {
                    sa_free(alloc, path_buf);
                    return -1;
                }
            }

            if (*cursor == '\0') break; // finished
            *cursor = '/'; // restore separator
        }
        cursor++;
    }

    sa_free(alloc, path_buf);
    return 0;
}

// Node for our directory stack
typedef struct {
    // Path is allocated immediately after this struct
    u8* path_start;
    u8* path_end; // length = bytesize(path_start, path_end)
} dir_node_t;

// Remove directory recursively using a custom stack (no recursion)
i32 directory_remove(stack_alloc* alloc, const u8* path_begin, const u8* path_end) {

    // Allocate initial stack for one entry
    dir_node_t** stack_begin = sa_alloc(alloc, sizeof(dir_node_t*));
    dir_node_t** stack_cursor = stack_begin;

    // Allocate root entry: structure + path
    uptr root_size = bytesize(path_begin, path_end);
    void* root_chunk = sa_alloc(alloc, sizeof(dir_node_t) + root_size + 1);

    dir_node_t* root_node = (dir_node_t*)root_chunk;
    root_node->path_start = (u8*)(root_node + 1); // path immediately after struct
    root_node->path_end   = byteoffset(root_node->path_start, root_size);
    memcpy(root_node->path_start, path_begin, root_size);
    root_node->path_start[root_size] = '\0';

    // Push root entry pointer onto stack
    *stack_cursor++ = root_node;

    while (stack_cursor != stack_begin) {
        dir_node_t* dir_node = *(--stack_cursor);

        DIR* dir = opendir((void*)dir_node->path_start);
        if (!dir) {
            unlink((void*)dir_node->path_start);
            sa_free(alloc, dir_node); // free chunk
            continue;
        }

        struct dirent* entry;
        int has_subdirs = 0;

        // Record cursor to roll back temporary entry paths per directory
        void* cursor_start = alloc->cursor;

        while ((entry = readdir(dir)) != NULL) {
            if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) continue;

            uptr entry_len = strlen(entry->d_name);
            uptr dir_len   = bytesize(dir_node->path_start, dir_node->path_end);

            // Allocate entry chunk: struct + path
            void* chunk = sa_alloc(alloc, sizeof(dir_node_t) + dir_len + 1 + entry_len + 1);
            dir_node_t* entry_node = (dir_node_t*)chunk;
            entry_node->path_start = (u8*)(entry_node + 1);
            entry_node->path_end   = byteoffset(entry_node->path_start, dir_len + 1 + entry_len);

            // Build path
            memcpy(entry_node->path_start, dir_node->path_start, dir_len);
            entry_node->path_start[dir_len] = '/';
            memcpy(byteoffset(entry_node->path_start, dir_len + 1), entry->d_name, entry_len);
            entry_node->path_start[dir_len + 1 + entry_len] = '\0';

            struct stat st;
            if (stat((void*)entry_node->path_start, &st) == 0) {
                if (S_ISDIR(st.st_mode)) {
                    // Push subdirectory entry onto stack
                    // Resize stack if necessary
                    uptr stack_bytes = bytesize(stack_begin, stack_cursor);
                    dir_node_t** new_stack = sa_alloc(alloc, stack_bytes + sizeof(dir_node_t*));
                    memcpy(new_stack, stack_begin, stack_bytes);
                    stack_cursor = new_stack + (stack_cursor - stack_begin);
                    stack_begin = new_stack;

                    *stack_cursor++ = entry_node;
                    has_subdirs = 1;
                } else {
                    unlink((void*)entry_node->path_start);
                    sa_free(alloc, entry_node);
                }
            } else {
                sa_free(alloc, entry_node);
            }
        }

        closedir(dir);

        // Rollback temporary allocation for this directory
        alloc->cursor = cursor_start;

        // Remove directory if no subdirectories remain
        if (!has_subdirs) {
            rmdir((void*)dir_node->path_start);
            sa_free(alloc, dir_node); // free chunk
        } else {
            *stack_cursor++ = dir_node; // re-push for deletion after children
        }
    }

    sa_free(alloc, stack_begin); // free stack array

    return 0;
}

// Get console file handles
file_t file_stdout(void) {
    return STDOUT_FILENO;
}

file_t file_stderr(void) {
    return STDERR_FILENO;
}

#include "file.h"
#include "assert.h"
#include "mem.h"
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <errno.h>
#include <dirent.h>

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
    sa_copy(alloc, path_begin, path, path_len);
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
uptr file_write(file_t file, const void* begin, const void* end) {
    uptr size = bytesize(begin, end);
    ssize_t bytes_written = write(file, begin, (size_t)size);
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

// Get modification time (milliseconds since epoch) for an open file handle.
uptr file_modification_time(file_t file) {
    debug_assert(file != file_invalid());
    struct stat st;
    if (fstat(file, &st) != 0) {
        return 0;
    }

    // Extract nanoseconds in a portable way:
    // On Linux/glibc st_mtim is available; on macOS use st_mtimespec.
#if defined(__APPLE__) || defined(__MACH__)
    long nsec = st.st_mtimespec.tv_nsec;
#elif defined(_POSIX_VERSION)
    long nsec = st.st_mtim.tv_nsec;
#else
    long nsec = 0;
#endif

    uptr secs = (uptr)st.st_mtime;
    uptr millis = secs * 1000ULL + (uptr)(nsec / 1000000L);
    return millis;
}

// Create directory recursively (like mkdir -p)
void directory_create(stack_alloc* alloc, const u8* path_begin, const u8* path_end, dir_mode_t mode) {

    // Allocate contiguous buffer for path
    uptr path_len = bytesize(path_begin, path_end);
    u8* path_buf = sa_alloc(alloc, path_len + 1);
    sa_copy(alloc, path_begin, path_buf, path_len);
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
                    goto end;
                }
            }

            *cursor = '/'; // restore separator
        }
        cursor++;
    }

end:
    sa_free(alloc, path_buf);
    return;
}

void directory_create_for_file(stack_alloc *alloc, const u8 *path_begin, const u8 *path_end, dir_mode_t mode) {
    if (!alloc || !path_begin || !path_end) {
        return;
    }

    uptr path_len = bytesize(path_begin, path_end);
    if (path_len == 0) {
        return;
    }

    // Strip trailing slashes (e.g., "dir/subdir/" -> treat as "dir/subdir")
    uptr len = path_len;
    while (len > 0 && *(u8*)(byteoffset(path_begin, len - 1)) == '/') {
        --len;
    }
    if (len == 0) {
        return;
    }

    // Find last slash in the trimmed path
    uptr last_slash = (uptr)-1;
    for (uptr i = len; i-- > 0;) {
        if (*(u8*)(byteoffset(path_begin, i)) == '/') {
            last_slash = i;
            break;
        }
    }

    // No directory component (just a filename) or only leading slash -> nothing to create
    if (last_slash == (uptr)-1 || last_slash == 0) {
        return;
    }

    // Create directories for the parent path portion
    const u8* dir_end = byteoffset(path_begin, last_slash);
    directory_create(alloc, path_begin, dir_end, mode);
}


// Node for our directory stack
typedef struct {
    void* previous;
    // Path is allocated immediately after this struct
    u8* path_start;
    u8* path_end; // length = bytesize(path_start, path_end)
} dir_node_t;

// Remove directory recursively using a custom stack (no recursion)
void directory_remove(stack_alloc* alloc, const u8* path_begin, const u8* path_end) {
    void* begin = alloc->cursor;

    // Allocate initial stack for one entry
    dir_node_t* stack_begin = alloc->cursor;
    dir_node_t* stack_cursor = stack_begin;

    // Allocate root entry: structure + path
    uptr root_size = bytesize(path_begin, path_end);
    dir_node_t* root_node = sa_alloc(alloc, sizeof(*root_node));
    root_node->previous = stack_begin - 1;
    root_node->path_start = sa_alloc(alloc,root_size);
    sa_copy(alloc, path_begin, root_node->path_start, root_size);
    root_node->path_end   = alloc->cursor;

    // Push root entry pointer onto stack
    // stack_cursor  = alloc->cursor;

    while (stack_cursor >= stack_begin) {
        dir_node_t* dir_node = stack_cursor;

        string dir_node_path_c;
        dir_node_path_c.begin = sa_alloc(alloc, bytesize(dir_node->path_start, dir_node->path_end) + 1);
        dir_node_path_c.end = alloc->cursor;
        sa_copy(alloc, dir_node->path_start, (void*)dir_node_path_c.begin, bytesize(dir_node->path_start, dir_node->path_end));
        *((u8*)dir_node_path_c.end - 1) = '\0';

        DIR* dir = opendir(dir_node_path_c.begin);
        if (!dir) {
            unlink(dir_node_path_c.begin);
            sa_free(alloc, (void*)dir_node_path_c.begin);
            sa_free(alloc, dir_node); // free chunk
            stack_cursor = stack_cursor->previous;
            continue;
        }
        sa_free(alloc, (void*)dir_node_path_c.begin);
        dir_node_path_c.begin = 0;
        dir_node_path_c.end = 0;

        struct dirent* entry;
        int has_subdirs = 0;

        while ((entry = readdir(dir)) != NULL) {
            const string entry_name = {entry->d_name, byteoffset(entry->d_name, mem_cstrlen(entry->d_name))};
            const uptr entry_len = bytesize(entry_name.begin, entry_name.end);
            const string dot = STR(".");
            const string dotdot = STR("..");
            if (sa_equals(alloc, entry_name.begin, entry_name.end, dot.begin, dot.end)
                || sa_equals(alloc, entry_name.begin, entry_name.end, dotdot.begin, dotdot.end)
                || sa_equals(alloc, entry_name.begin, entry_name.end, 
                        byteoffset(dir_node->path_end, -bytesize(entry_name.begin, entry_name.end) - 1), dir_node->path_end)) {
                continue;
            }
            uptr dir_len   = bytesize(dir_node->path_start, dir_node->path_end);

            // Allocate entry chunk: struct + path
            dir_node_t* entry_node = sa_alloc(alloc, sizeof(*entry_node));
            entry_node->previous = dir_node;
            entry_node->path_start = alloc->cursor;

            // Build path
            void* cursor = sa_alloc(alloc, dir_len);
            sa_copy(alloc, dir_node->path_start, cursor, dir_len);
            *(u8*)sa_alloc(alloc, 1) = '/'; 
            cursor = sa_alloc(alloc, entry_len);
            sa_copy(alloc, entry->d_name, cursor, entry_len);
            entry_node->path_end = alloc->cursor;

            dir_node_path_c.begin = sa_alloc(alloc, bytesize(entry_node->path_start, entry_node->path_end) + 1);
            dir_node_path_c.end = alloc->cursor;
            sa_copy(alloc, entry_node->path_start, (void*)dir_node_path_c.begin, bytesize(entry_node->path_start, entry_node->path_end));
            *((u8*)dir_node_path_c.end - 1) = '\0';
            struct stat st;
            if (stat(dir_node_path_c.begin, &st) == 0) {
                if (S_ISDIR(st.st_mode)) {
                    // Push subdirectory entry onto stack
                    stack_cursor = entry_node;
                    has_subdirs = 1;
                    sa_free(alloc, (void*)dir_node_path_c.begin);
                } else {
                    unlink(dir_node_path_c.begin);
                    sa_free(alloc, (void*)dir_node_path_c.begin);
                    sa_free(alloc, entry_node);
                }
            } else {
                sa_free(alloc, (void*)dir_node_path_c.begin);
                sa_free(alloc, entry_node);
            }
        }

        closedir(dir);

        // Remove directory if no subdirectories remain
        if (!has_subdirs) {
            dir_node_path_c.begin = sa_alloc(alloc, bytesize(dir_node->path_start, dir_node->path_end) + 1);
            dir_node_path_c.end = alloc->cursor;
            rmdir(dir_node_path_c.begin);
            sa_free(alloc, (void*)dir_node_path_c.begin);
            sa_free(alloc, dir_node); // free chunk
            stack_cursor = stack_cursor->previous;
        }
    }

    sa_free(alloc, stack_begin); // free stack array

    debug_assert(alloc->cursor == begin);
}

void directory_parent(const u8* path_begin, const u8* path_end, u8** out_begin, u8** out_end) {
    // Validate outputs
    if (!out_begin || !out_end) return;

    // Default to empty slice
    *out_begin = (u8*)path_begin;
    *out_end = (u8*)path_begin;

    if (!path_begin || !path_end) return;

    uptr len = bytesize(path_begin, path_end);
    if (len == 0) {
        return;
    }

    // Trim trailing slashes but keep a single leading slash for root
    uptr trimmed = len;
    while (trimmed > 1 && *(u8*)byteoffset(path_begin, trimmed - 1) == '/') {
        --trimmed;
    }

    if (trimmed == 0) {
        return;
    }

    // Find last slash in the trimmed range
    uptr last_slash = (uptr)-1;
    for (uptr i = trimmed; i-- > 0;) {
        if (*(u8*)byteoffset(path_begin, i) == '/') {
            last_slash = i;
            break;
        }
    }

    if (last_slash == (uptr)-1) {
        // No slash found: no parent directory
        *out_begin = (u8*)path_begin;
        *out_end = (u8*)path_begin;
        return;
    }

    if (last_slash == 0) {
        // Parent is the root "/"
        *out_begin = (u8*)path_begin;
        *out_end = (u8*)byteoffset(path_begin, 1);
        return;
    }

    // Parent is everything up to and including the last slash (ensure trailing '/')
    *out_begin = (u8*)path_begin;
    *out_end = (u8*)byteoffset(path_begin, last_slash + 1);
}

// Get console file handles
file_t file_stdout(void) {
    return STDOUT_FILENO;
}

file_t file_stderr(void) {
    return STDERR_FILENO;
}

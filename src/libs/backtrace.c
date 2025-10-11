#include "backtrace.h"
#include "primitive.h"
#include "print.h"
#include "file.h"
#include "stack_alloc.h"
#include "exec_command.h"

#include <execinfo.h>
#include <stdio.h>
#include <unistd.h>

void print_backtrace(file_t file) {
    void *buffer[64];
    i32 nptrs = backtrace(buffer, 64);

    u8 exe_path[1024];
    iptr len = readlink("/proc/self/exe", (char*)exe_path, sizeof(exe_path) - 1);
    if (len == -1) {
        perror("readlink");
        return;
    }
    string exe_path_str = {exe_path, byteoffset(exe_path, len)};

    print_string(file, STRING("Backtrace (most recent call last):\n") );

    for (i32 i = 0; i < nptrs; i++) {
        u8 cmd[2048];
        stack_alloc alloc;
        sa_init(&alloc, cmd, byteoffset(cmd, sizeof(cmd)));
        print_format_to_buffer(&alloc, STRING("addr2line -e %s -f -C -i %p\0"), exe_path_str, buffer[i]);
        
        u8* cmd_end = cmd;
        while (*cmd_end) { ++cmd_end; }
        string cmd_str = { cmd, cmd_end };

        string result;
        result.begin = exec_command(cmd_str, &alloc);
        result.end = alloc.cursor;

        {
            const u8* cursor = result.begin;
            const u8* line_begin = cursor;
            while (1) {
                if (cursor == result.end || *cursor == '\n') {
                    const u8* line_end = cursor;
                    if (bytesize(line_begin, line_end) > 0) {
                        print_format(file, STRING("  [%d] %s\n"), i, (string){line_begin, line_end});
                    }
                    line_begin = byteoffset(cursor, 1);
                }
                
                if (cursor == result.end) {
                    break;
                }

                cursor = byteoffset(cursor, 1);
            }
        }
        
        sa_free(&alloc, cmd);
        sa_deinit(&alloc);
    }
}

#include "backtrace.h"
#include "primitive.h"
#include "print.h"
#include "file.h"

#include <execinfo.h>
#include <stdio.h>
#include <unistd.h>

void print_backtrace(void) {
    void *buffer[64];
    i32 nptrs = backtrace(buffer, 64);

    u8 exe_path[1024];
    iptr len = readlink("/proc/self/exe", (char*)exe_path, sizeof(exe_path) - 1);
    if (len == -1) {
        perror("readlink");
        return;
    }
    string exe_path_str = {exe_path, byteoffset(exe_path, len)};

    print_string(file_stderr(), STRING("Backtrace (most recent call last):\n") );

    for (i32 i = 0; i < nptrs; i++) {
        u8 cmd[2048];
        stack_alloc alloc;
        sa_init(&alloc, cmd, byteoffset(cmd, sizeof(cmd)));
        print_format_to_buffer(&alloc, STRING("addr2line -e %s -f -C -i %p"), exe_path_str, buffer[i]);

        FILE *fp = popen((char*)cmd, "r");
        if (fp) {
            u8 line[1024];
            while (1) {
                char* end = fgets((char*)line, sizeof(line), fp);
                if (!end) {break;}
                print_format(file_stdout(), STRING("  [%d] %s"), (string){line, end}, line);
            }
            pclose(fp);
        } else {
            print_format(file_stdout(), STRING("  [%d] %p\n"), i, buffer[i]);
        }

        sa_free(&alloc, cmd);
        sa_deinit(&alloc);
    }
}

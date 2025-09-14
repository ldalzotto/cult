#include "./backtrace.h"
#include "./primitive.h"
#include "./print.h"
#include "./file.h"

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
    exe_path[len] = '\0';

    print_string("Backtrace (most recent call last):\n", file_get_stderr());

    for (i32 i = 0; i < nptrs; i++) {
        u8 cmd[2048];
        // TODO: remove that
        snprintf((char*)cmd, sizeof(cmd),
                 "addr2line -e %s -f -C -i %p",
                 exe_path, buffer[i]);

        FILE *fp = popen((char*)cmd, "r");
        if (fp) {
            u8 line[1024];
            while (fgets((char*)line, sizeof(line), fp)) {
                print_format(file_get_stdout(), "  [%d] %s", i, line);
            }
            pclose(fp);
        } else {
            print_format(file_get_stdout(), "  [%d] %p\n", i, buffer[i]);
        }
    }
}

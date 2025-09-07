#include "./backtrace.h"
#include "./primitive.h"

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

    printf("Backtrace (most recent call last):\n");

    for (i32 i = 0; i < nptrs; i++) {
        u8 cmd[2048];
        snprintf((char*)cmd, sizeof(cmd),
                 "addr2line -e %s -f -C -i %p",
                 exe_path, buffer[i]);

        FILE *fp = popen((char*)cmd, "r");
        if (fp) {
            u8 line[1024];
            while (fgets((char*)line, sizeof(line), fp)) {
                printf("  [%d] %s", i, line);
            }
            pclose(fp);
        } else {
            printf("  [%d] %p\n", i, buffer[i]);
        }
    }
}
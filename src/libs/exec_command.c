
#include "exec_command.h"

#include <stdio.h>
#include <sys/wait.h>
#include "mem.h"
#include "assert.h"

exec_command_result exec_command(const string cmd, stack_alloc* alloc) {
    exec_command_result result;
    result.output = alloc->cursor;
    result.success = 0;

    string cmd_null_terminated = cmd;
    if (*((u8*)cmd.end - 1) != '\0') {
        cmd_null_terminated.begin = sa_alloc(alloc, bytesize(cmd.begin, cmd.end) + 1);
        cmd_null_terminated.end = alloc->cursor;
        sa_copy(alloc, cmd.begin, (void*)cmd_null_terminated.begin, bytesize(cmd.begin, cmd.end));
        *((u8*)cmd_null_terminated.end - 1) = '\0';
    }

    FILE *fp = popen(cmd_null_terminated.begin, "r");
    if (!fp) {
        if (cmd_null_terminated.begin != cmd.begin) {
            sa_free(alloc, (void*)cmd_null_terminated.begin);
        }
        return result;
    }

    string command = {alloc->cursor, alloc->cursor};
    while (fgets(alloc->cursor, bytesize(alloc->cursor, alloc->end), fp)) {
        const uptr size = mem_cstrlen(alloc->cursor);
        alloc->cursor = byteoffset(alloc->cursor, size);
        debug_assert(alloc->cursor <= alloc->end);
        command.end = alloc->cursor;
    }

    if (cmd_null_terminated.begin != cmd.begin) {
        sa_move_tail(alloc, (void*)command.begin, (void*)cmd_null_terminated.begin);
    }

    i32 wstat = pclose(fp);
    if (wstat == -1) {
        /* pclose error */
        result.success = 0;
    } else {
        if (WIFEXITED(wstat)) {
            i32 exit_code = WEXITSTATUS(wstat);
            result.success = (exit_code == 0) ? 1 : 0;
        } else {
            /* terminated by signal or other non-exit condition */
            result.success = 0;
        }
    }

    return result;
}


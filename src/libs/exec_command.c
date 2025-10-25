
#include "exec_command.h"

#include <sys/wait.h>
#include <unistd.h>

exec_command_result exec_command(const string cmd, stack_alloc* alloc) {
    void* begin = alloc->cursor;
    exec_command_session* session = open_persistent_shell(alloc);
    exec_command_result result = command_session_exec_command(session, cmd, alloc);
    close_persistent_shell(session);
    sa_move_tail(alloc, result.output, begin);
    result.output = begin;
    return result;
}

struct exec_command_session {
    int in_fd;   // write commands here
    int out_fd;  // read results here
    pid_t pid;
};

exec_command_session* open_persistent_shell(stack_alloc* alloc) {
    exec_command_session* session = sa_alloc(alloc, sizeof(*session));
    *session = (exec_command_session){ .in_fd = -1, .out_fd = -1, .pid = -1 };
    int inpipe[2], outpipe[2];
    if (pipe(inpipe) != 0 || pipe(outpipe) != 0) return session;

    pid_t pid = fork();
    if (pid == 0) {
        dup2(inpipe[0], STDIN_FILENO);
        dup2(outpipe[1], STDOUT_FILENO);
        dup2(outpipe[1], STDERR_FILENO);
        close(inpipe[0]); close(inpipe[1]);
        close(outpipe[0]); close(outpipe[1]);
        execl("/bin/sh", "sh", "-s", (char*)NULL);   // interactive shell
        _exit(127);
    }

    close(inpipe[0]);
    close(outpipe[1]);
    session->in_fd  = inpipe[1];
    session->out_fd = outpipe[0];
    session->pid    = pid;
    return session;
}

exec_command_result command_session_exec_command(exec_command_session* session, const string cmd, stack_alloc* alloc) {
    
    exec_command_result result;
    result.output = alloc->cursor;
    result.success = 0;

    void* cursor;
    string command;
    command.begin = alloc->cursor;
    cursor = sa_alloc(alloc, bytesize(cmd.begin, cmd.end));
    sa_copy(alloc, cmd.begin, cursor, bytesize(cmd.begin, cmd.end));

    const string end_marker_command = STR("; echo __END__ $?\n");
    cursor = sa_alloc(alloc, bytesize(end_marker_command.begin, end_marker_command.end));
    sa_copy(alloc, end_marker_command.begin, cursor, bytesize(end_marker_command.begin, end_marker_command.end));
    command.end = alloc->cursor;

    write(session->in_fd, command.begin, bytesize(command.begin, command.end));
    fsync(session->in_fd);

    string out = {alloc->cursor, alloc->cursor};
    while (1) {
        void* begin = alloc->cursor;
        const uptr size = read(session->out_fd, alloc->cursor, bytesize(alloc->cursor, alloc->end));
        alloc->cursor = byteoffset(alloc->cursor, size);
        if (size > 0) {
            const string end_marker = STR("__END__");
            void* marker_start = sa_find(alloc, begin, alloc->cursor, end_marker.begin, end_marker.end);
            if (marker_start) {
                void* marker_end = byteoffset(marker_start, bytesize(end_marker.begin, end_marker.end));
                u8* code_start = byteoffset(marker_end, 1);

                sa_free(alloc, marker_start);
                result.success = *code_start == '0';
                break;
            }
        }
    }
    
    out.end = alloc->cursor;
    
    sa_move_tail(alloc, (void*)out.begin, (void*)command.begin);

    return result;
}

void close_persistent_shell(exec_command_session* session) {
    const string exit_command = STR("exit\n");
    write(session->in_fd, exit_command.begin, bytesize(exit_command.begin, exit_command.end));
    close(session->in_fd);
    close(session->out_fd);
    waitpid(session->pid, NULL, 0);
}

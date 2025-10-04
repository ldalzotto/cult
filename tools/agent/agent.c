#include "primitive.h"
#include "file.h"
#include "mem.h"
#include "litteral.h"
#include "user_content_read.h"
#include "agent_result_write.h"
#include "agent_request.h"
#include "print.h"

i32 main(i32 argc, char** argv) {

    uptr size = 1024 * 1024;
    void* memory = mem_map(size);
    stack_alloc _alloc;
    stack_alloc* alloc = &_alloc;
    sa_init(alloc, memory, byteoffset(memory, size));

    string file_path;
    if (argc < 2) {
        sa_deinit(alloc);
        mem_unmap(memory, size);
        return 1;
    }
    file_path.begin = (u8*)argv[1];
    file_path.end = (u8*)argv[1] + mem_cstrlen(argv[1]);

    // Read the file
    file_t file = file_open(alloc, file_path.begin, file_path.end, FILE_MODE_READ_WRITE);
    u8_slice file_content;
    file_read_all(file, (void**)&file_content.begin, alloc);
    file_content.end = alloc->cursor;

    u8_slice user_content = user_content_read(alloc, file_content);
    u8_slice agent_result;
    agent_result.begin = agent_request(user_content, alloc);
    agent_result.end = alloc->cursor;

    print_format(file_stdout(), STRING("%s\n"), agent_result);

    agent_result_write(file, alloc, agent_result);
    file_close(file);

    sa_free(alloc, agent_result.begin);
    sa_free(alloc, file_content.begin);

    sa_deinit(alloc);
    mem_unmap(memory, size);

    return 0;
}

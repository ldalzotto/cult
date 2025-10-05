#include "primitive.h"
#include "file.h"
#include "mem.h"
#include "litteral.h"
#include "user_content_read.h"
#include "agent_result_write.h"
#include "agent_request.h"

i32 main(i32 argc, char** argv) {

    uptr size = 1024 * 1024 * 10;
    void* memory = mem_map(size);
    stack_alloc _alloc;
    stack_alloc* alloc = &_alloc;
    sa_init(alloc, memory, byteoffset(memory, size));

    string file_path;

    string api_key;
    api_key.begin = 0;
    api_key.end = 0;

    for (i32 i = 1; i < argc; ++i) {
        const char* arg = (const char*)argv[i];

        // --key value
        if (arg[0] == '-' && arg[1] == '-' && arg[2] == 'k' && arg[3] == 'e' && arg[4] == 'y' && arg[5] == '\0') {
            if (i + 1 < argc) {
                const char* val = argv[i + 1];
                api_key.begin = (u8*)val;
                api_key.end = (u8*)val + mem_cstrlen((void*)val);
                i++;
                break;
            }
        }
    }

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

    u8_slice user_content;
    user_content.begin = user_content_read(alloc, file_content);
    user_content.end = alloc->cursor;

    sa_move_tail(alloc, user_content.begin, file_content.begin);
    const uptr user_content_size = bytesize(user_content.begin, user_content.end);
    user_content.begin = file_content.begin;
    user_content.end = byteoffset(user_content.begin, user_content_size);
    file_content.begin = 0;file_content.end = 0;

    u8_slice agent_result;
    agent_result.begin = agent_request(user_content, api_key, alloc);
    agent_result.end = alloc->cursor;

    agent_result_write(file, alloc, agent_result);
    file_close(file);

    sa_free(alloc, agent_result.begin);
    sa_free(alloc, user_content.begin);

    sa_deinit(alloc);
    mem_unmap(memory, size);

    return 0;
}

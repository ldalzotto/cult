#include "agent_result_write.h"

static u8* format_agent_result(u8_slice agent_result, stack_alloc* alloc) {
    void* begin = alloc->cursor;
    u8* p = agent_result.begin;
    u8* e = agent_result.end;

    for (; p < e; ++p) {
        u8 c = *p;
        if (c == '\\') {
            if (p + 1 < e) {
                u8 next = *(p + 1);
                if (next == 'n') {
                    ((u8*)alloc->cursor)[0] = '\n';
                    alloc->cursor = byteoffset(alloc->cursor, 1);
                    ++p; // consume 'n'
                    continue;
                } else if(next == '\\') {
                    ((u8*)alloc->cursor)[0] = '\\';
                    alloc->cursor = byteoffset(alloc->cursor, 1);
                    ++p; // consume '\'
                    continue;
                } else if (next == '"') {
                    ((u8*)alloc->cursor)[0] = '"';
                    alloc->cursor = byteoffset(alloc->cursor, 1);
                    ++p; // consume '"'
                    continue;
                }
            }
            // Copy lone backslash or unsupported escape as-is
            ((u8*)alloc->cursor)[0] = c;
            alloc->cursor = byteoffset(alloc->cursor, 1);
        } else {
            ((u8*)alloc->cursor)[0] = c;
            alloc->cursor = byteoffset(alloc->cursor, 1);
        }
    }

    return begin;
}


void agent_result_write(file_t file, stack_alloc* alloc, u8_slice agent_result) {
    void* begin = alloc->cursor;

    u8_slice agent_request_formatted;
    agent_request_formatted.begin = format_agent_result(agent_result, alloc);
    agent_request_formatted.end = alloc->cursor;

    static const u8 tag[] = "\n[AGENT]\n\n";
    file_write(file, tag, tag + sizeof(tag) - 1);

    file_write(file, agent_request_formatted.begin, agent_request_formatted.end);

    sa_free(alloc, begin);
}

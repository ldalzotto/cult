#include "agent_request.h"
#include <stdio.h>
#include <stdlib.h>
#include "print.h"
#include "mem.h"

static u8* format_user_content(u8_slice user_content, stack_alloc* alloc) {
    void* begin = alloc->cursor;
    u8* p = user_content.begin;
    u8* e = user_content.end;
    for (; p < e; ++p) {
        u8 c = *p;
        if (c == '\n') {
            ((u8*)alloc->cursor)[0] = '\\';
            ((u8*)alloc->cursor)[1] = 'n';
            alloc->cursor = byteoffset(alloc->cursor, 2);
        } else if (c == '"') {
            ((u8*)alloc->cursor)[0] = '\\';
            ((u8*)alloc->cursor)[1] = '"';
            alloc->cursor = byteoffset(alloc->cursor, 2);
        } else {
            ((u8*)alloc->cursor)[0] = c;
            alloc->cursor = byteoffset(alloc->cursor, 1);
        }
    }

    return begin;
}

static u8_slice extract_answer(u8_slice api_output) {
    u8_slice answer;
    answer.begin = api_output.begin;
    answer.end = api_output.begin;

    /*[USER]
        Find \"output\"
        From this position, find the first \"text\"
        Then return the slice corresponding to the message.
            Example: "text": "print(\"This is the second user tag\")"
    */

    const u8* begin = api_output.begin;
    const u8* end = api_output.end;

    // Patterns to search
    const u8 pat_output[] = "\"output\"";
    const size_t pat_output_len = sizeof(pat_output) - 1;
    const u8 pat_text[] = "\"text\"";
    const size_t pat_text_len = sizeof(pat_text) - 1;

    // Find "output"
    const u8* cur = begin;
    const u8* pos_output = end;
    while (cur < end) {
        if ((size_t)(end - cur) < pat_output_len) break;
        size_t i = 0;
        while (i < pat_output_len && cur[i] == pat_output[i]) ++i;
        if (i == pat_output_len) { pos_output = cur; break; }
        ++cur;
    }
    if (pos_output == end) {
        return answer;
    }

    // Find "text" after "output"
    cur = pos_output + pat_output_len;
    const u8* pos_text = end;
    while (cur < end) {
        if ((size_t)(end - cur) < pat_text_len) break;
        size_t i = 0;
        while (i < pat_text_len && cur[i] == pat_text[i]) ++i;
        if (i == pat_text_len) { pos_text = cur; break; }
        ++cur;
    }
    if (pos_text == end) {
        return answer;
    }

    // Move to the value after "text": "...", allowing spaces
    cur = pos_text + pat_text_len;
    while (cur < end && *cur != ':') ++cur;
    if (cur == end) return answer;
    ++cur; // after ':'

    // Skip whitespace
    while (cur < end && (*cur == ' ' || *cur == '\t' || *cur == '\r' || *cur == '\n')) ++cur;
    if (cur == end) return answer;

    // Find opening quote of the JSON string value
    while (cur < end && *cur != '"') ++cur;
    if (cur == end) return answer;
    ++cur; // move past opening quote

    const u8* text_begin = cur;

    // Find closing quote, handling escapes
    while (cur < end) {
        if (*cur == '\\') {
            ++cur;
            if (cur < end) ++cur; // skip escaped character
            continue;
        }
        if (*cur == '"') {
            break;
        }
        ++cur;
    }
    if (cur == end) return answer;

    answer.begin = (u8*)text_begin;
    answer.end = (u8*)cur;

    return answer;
}


#if 1
static u8* run_command(u8_slice command, stack_alloc* alloc) {
    FILE* pipe = popen((const char*)command.begin, "r");
    sa_free(alloc, command.begin);
    if (!pipe) {
        sa_free(alloc, alloc->cursor);
        return alloc->cursor;
    }

    void* response_begin = alloc->cursor;
    const uptr chunk_size = 4096;
    for (;;) {
        size_t r = fread(alloc->cursor, 1, chunk_size, pipe);
        if (r > 0) {
            alloc->cursor = byteoffset(alloc->cursor, r);
        }
        if (feof(pipe) || ferror(pipe)) break;
    }

    pclose(pipe);

    return response_begin;
}
#endif

#if 0

static u8* run_command(u8_slice command, stack_alloc* alloc) {
    unused(command);
    void* begin = alloc->cursor;
    string path = STR("/home/a/Documents/Dev/cult/mock.json");
    file_t file = file_open(alloc, path.begin, path.end, FILE_MODE_READ);
    void* tmp;
    file_read_all(file, &tmp, alloc);
    file_close(file);
    return begin;
}

#endif

u8* agent_request(u8_slice user_content, stack_alloc* alloc) {
    void* begin = alloc->cursor;

    const char* api_key_cstr = getenv("OPENAI_API_KEY");
    if (!api_key_cstr) {
        return alloc->cursor;
    }
    const string api_key = {api_key_cstr, byteoffset(api_key_cstr, mem_cstrlen((void*)api_key_cstr))};

    u8_slice user_content_formatted;
    user_content_formatted.begin = format_user_content(user_content, alloc);
    user_content_formatted.end = alloc->cursor;

    const string culr = STR(
        "curl -s https://api.openai.com/v1/responses "
        "-H \"Content-Type: application/json\" "
        "-H \"Authorization: Bearer %s\" "
        "-d '{"
            "\"model\":\"gpt-5-nano\","
            "\"prompt\":{"
                "\"id\":\"pmpt_68dab642df5c8193b611a4b526c3661d050b6e8ebdd8c5c0\","
                "\"version\":\"4\""
            "},"
            "\"input\": \"%s\""
        "}'"
        "\0"
    );

    u8_slice cmd;
    cmd.begin = print_format_to_buffer(alloc, culr, api_key, user_content_formatted);
    cmd.end = alloc->cursor;

    print_format(file_stdout(), STRING("%s\n"), cmd);

    void* response_begin = run_command(cmd, alloc);

    sa_move_tail(alloc, response_begin, begin);

    u8_slice answer = extract_answer((u8_slice){begin, alloc->cursor});
    sa_move_tail(alloc, answer.begin, begin);
    sa_free(alloc, byteoffset(begin, bytesize(answer.begin, answer.end)));

    return begin;
}

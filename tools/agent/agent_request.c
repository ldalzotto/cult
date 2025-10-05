#include "agent_request.h"
#include <stdio.h>
#include <stdlib.h>

#include "assert.h"
#include "network/https/https_request.h"

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
        } else if (c == '\\' && *(p+1) != 'n') {
            ((u8*)alloc->cursor)[0] = '\\';
            ((u8*)alloc->cursor)[1] = '\\';
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

/* ------------------------- Agent request ------------------------- */

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

    // Build JSON body (was curl -d before)
    const string json_tpl = STR(
        "{"
            "\"model\":\"gpt-5-nano\","
            "\"prompt\":{"
                "\"id\":\"pmpt_68dab642df5c8193b611a4b526c3661d050b6e8ebdd8c5c0\","
                "\"version\":\"4\""
            "},"
            "\"input\":\"%s\""
        "}"
    );

    string json_body;
    json_body.begin = print_format_to_buffer(alloc, json_tpl, user_content_formatted);
    json_body.end = alloc->cursor;
    const uptr body_size = bytesize(json_body.begin, json_body.end);

    // Perform HTTPS POST using OpenSSL and write response body into allocator
    string url = STR("api.openai.com");
    string port = STR("443");

    string post_body_template = STR(
        "POST /v1/responses HTTP/1.1\r\n"
        "Host: api.openai.com\r\n"
        "Content-Type: application/json\r\n"
        "Connection: close\r\n"
        "Authorization: Bearer %s\r\n"
        "Content-Length: %u\r\n"
        "\r\n"
    );

    string header;
    header.begin = print_format_to_buffer(alloc, post_body_template, api_key, body_size);
    header.end = alloc->cursor;

    /* Inverse memory */
    const uptr header_size = bytesize(header.begin, header.end);
    void* header_move_begin = sa_alloc(alloc, header_size); // give enough space
    sa_move(alloc, (void*)header.begin, header_move_begin, header_size);
    header.begin = header_move_begin;  
    header.end = byteoffset(header_move_begin, header_size);
    
    void* body_move_begin = byteoffset(json_body.begin, header_size);
    sa_move(alloc, (void*)json_body.begin, body_move_begin, body_size);
    header_move_begin = (void*)json_body.begin;
    json_body.begin = body_move_begin;
    json_body.end = byteoffset(body_move_begin, body_size);

    sa_move(alloc, (void*)header.begin, header_move_begin, header_size);
    header.begin = header_move_begin;
    header.end = byteoffset(header_move_begin, header_size);

    debug_assert(header.end == json_body.begin);
    
    void* response_begin = https_request_sync(alloc, 
        (u8_slice){(void*)url.begin, (void*)url.end}, 
        (u8_slice){(void*)port.begin, (void*)port.end},
        (u8_slice){(void*)header.begin, (void*)json_body.end});

    // Move response to the beginning of the region used by this function
    sa_move_tail(alloc, response_begin, begin);

    // Extract the answer
    u8_slice answer = extract_answer((u8_slice){begin, alloc->cursor});
    sa_move_tail(alloc, answer.begin, begin);
    sa_free(alloc, byteoffset(begin, bytesize(answer.begin, answer.end)));

    return begin;
}

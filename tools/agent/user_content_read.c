#include "user_content_read.h"

#include "litteral.h"
#include "file.h"
#include "print.h"

u8* user_content_read(stack_alloc* alloc, u8_slice file_content) {
    void* begin = alloc->cursor;
    u8_slice user_content;
    {
        const string pattern = STR("[USER]");
        const uptr pattern_size = bytesize(pattern.begin, pattern.end);

        const u8* last = file_content.end;
        const u8* cur = file_content.begin;

        while (cur != file_content.end) {
            uptr remaining = bytesize(cur, file_content.end);
            if (remaining < pattern_size) {
                break;
            }
            if (sa_equals(alloc, cur, byteoffset(cur, pattern_size), pattern.begin, pattern.end)) {
                last = cur;
                cur = byteoffset(cur, pattern_size);
            } else {
                ++cur;
            }
        }

        if (last == file_content.end) {
            user_content.begin = file_content.end;
            user_content.end = file_content.end;
        } else {
            user_content.begin = byteoffset(last, pattern_size);
            user_content.end = file_content.end;
        }
    }

    sa_alloc_copy(alloc, user_content.begin, user_content.end);

    const char token[] = "@";
    const uptr token_len = sizeof(token) - 1;

    const u8* cur = user_content.begin;
    while (cur != user_content.end) {
        uptr remaining = bytesize(cur, user_content.end);
        if (remaining < token_len) {
            break;
        }

        if (sa_equals(alloc, cur, byteoffset(cur, token_len), token, token + token_len)) {
            cur = byteoffset(cur, token_len);

            // skip whitespace after the token
            while (cur != user_content.end) {
                unsigned char c = *cur;
                if (c != ' ' && c != '\t' && c != '\n' && c != '\r') break;
                ++cur;
            }

            const u8* path_begin = cur;
            // read until whitespace or quote or end
            while (cur != user_content.end) {
                unsigned char c = *cur;
                if (c == ' ' || c == '\t' || c == '\n' || c == '\r' || c == '"' || c == '\'') break;
                ++cur;
            }
            const u8* path_end = cur;
            file_t file = file_open(alloc, path_begin, path_end, FILE_MODE_READ);
            if (file != file_invalid()) {
                /* <file path> </file> */
                const string file_open_template = STR("<file %s>\n");
                const string file_close_template = STR("\n</file>\n");
                print_format_to_buffer(alloc, file_open_template, (string){path_begin, path_end});
                void* tmp;
                file_read_all(file, &tmp, alloc);
                print_format_to_buffer(alloc, file_close_template);
                file_close(file);
            }
        } else {
            ++cur;
        }
    }
    
    return begin;
}

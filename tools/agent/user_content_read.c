#include "user_content_read.h"

#include "litteral.h"

u8_slice user_content_read(stack_alloc* alloc, u8_slice file_content) {
    u8_slice user_content;
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
    return user_content;
}

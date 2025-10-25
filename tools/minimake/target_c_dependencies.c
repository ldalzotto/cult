#include "primitive.h"
#include "stack_alloc.h"
#include "target_c_dependencies.h"

string* extract_c_dependencies(string gcc_output, stack_alloc* alloc) {
    void* begin = alloc->cursor;
    
    string* deps_start = alloc->cursor;
    
    // mark the start of the deps array in the allocator so callers can store it
    deps_start = alloc->cursor;
    // track whether we've skipped the "target:" token
    u8 skipped_target = 0;

    char* p = (char*)gcc_output.begin;
    char* e = (char*)gcc_output.end;
    while (p < e) {
        // skip whitespace and backslashes (line continuations)
        while (p < e && (*p == ' ' || *p == '\t' || *p == '\n' || *p == '\r' || *p == '\\')) { p++; }
        if (p >= e) break;

        char* tok_b = p;
        // token continues until whitespace or backslash
        while (p < e && !(*p == ' ' || *p == '\t' || *p == '\n' || *p == '\r' || *p == '\\')) { p++; }
        char* tok_e = p;

        if (tok_e == tok_b) continue;

        // If this is the first token and it ends with ':', skip it (it's the target)
        if (!skipped_target && tok_e > tok_b && tok_e[-1] == ':') {
            skipped_target = 1;
            continue;
        }

        // If token ends with ':' (unexpected here), trim it
        if (tok_e > tok_b && tok_e[-1] == ':') {
            tok_e--;
            if (tok_e == tok_b) continue;
        }

        // Allocate a string entry for this dependency
        string* dep_entry = sa_alloc(alloc, sizeof(*dep_entry));
        dep_entry->begin = sa_alloc(alloc, bytesize(tok_b, tok_e));
        sa_copy(alloc, tok_b, (void*)dep_entry->begin, bytesize(tok_b, tok_e));
        dep_entry->end = alloc->cursor;
    }

    string* deps_end = alloc->cursor;

    const uptr offset = bytesize(begin, deps_start);
    sa_move_tail(alloc, deps_start, begin);

    deps_start = byteoffset(deps_start, -offset);
    deps_end = byteoffset(deps_end, -offset);

    for (string* d = deps_start; d < deps_end;) {
        d->begin = byteoffset(d->begin, -offset);
        d->end = byteoffset(d->end, -offset);
        d = (void*)d->end;
    }

    return begin;
}

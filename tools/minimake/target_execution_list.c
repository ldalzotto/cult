#include "primitive.h"
#include "stack_alloc.h"
#include "target_execution_list.h"
#include "target.h"

target** target_execution_list(target* targets_begin, target* targets_end, target* main, stack_alloc* alloc) {

    /*
        Build a list of unique targets to execute, starting with 'main' as the
        first element. The list is stored in the allocator as an array of
        target* entries. Entries are appended as dependencies are discovered.
        The caller expects the list to be such that iterating from end-1 down
        to begin executes dependencies first and the main target last.
    */

    /* Reserve the start of the list in the allocator and push 'main' */
    target** list_begin = alloc->cursor;
    target** first = sa_alloc(alloc, sizeof(*first));
    *first = main;
    target** list_end = alloc->cursor;

    /* Iterate over the (growing) list and discover target dependencies */
    for (target** it = list_begin; it < list_end; ++it) {
        target* cur = *it;

        /* Iterate dependencies of cur */
        for (string* d = cur->deps; (void*)d < cur->end; ) {

            /* Try to find a target whose name matches this dependency */
            for (target* cand = targets_begin; cand < targets_end; cand = cand->end) {
                uptr cand_len = bytesize(cand->name.begin, cand->name.end);
                uptr dep_len = bytesize(d->begin, d->end);

                /* Quick length check */
                if (cand_len != dep_len) { /* not equal */ }
                else {
                    /* Compare bytes */
                    u8 equal = 1;
                    {
                        unsigned char* a = (unsigned char*)cand->name.begin;
                        unsigned char* b = (unsigned char*)d->begin;
                        uptr i;
                        for (i = 0; i < cand_len; ++i) {
                            if (a[i] != b[i]) { equal = 0; break; }
                        }
                    }

                    if (equal) {
                        /* Ensure uniqueness: check if cand is already in list by name */
                        u8 already = 0;
                        for (target** check = list_begin; check < list_end; ++check) {
                            target* existing = *check;
                            uptr exist_len = bytesize(existing->name.begin, existing->name.end);
                            if (exist_len != cand_len) { continue; }
                            u8 same = 1;
                            {
                                unsigned char* a = (unsigned char*)existing->name.begin;
                                unsigned char* b = (unsigned char*)cand->name.begin;
                                uptr i;
                                for (i = 0; i < cand_len; ++i) {
                                    if (a[i] != b[i]) { same = 0; break; }
                                }
                            }
                            if (same) { already = 1; break; }
                        }

                        if (!already) {
                            target** push = sa_alloc(alloc, sizeof(*push));
                            *push = cand;
                            list_end = alloc->cursor;
                        } else {
                            // [TASK] Move the already found target at the end of the array.
                        }

                        /* Found matching target, no need to check other candidates */
                        break;
                    }
                }
            }

            d = (void*)d->end;
        }
    }

    return list_begin;
}

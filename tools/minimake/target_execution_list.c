#include "primitive.h"
#include "stack_alloc.h"
#include "target_execution_list.h"
#include "target.h"

static target** find_last_target_with_same_name(target** targets_begin, target** targets_end, string name, stack_alloc* alloc) {
    for (target** check = targets_begin; check < targets_end; ++check) {
        target* existing = *check;
        u8 same = sa_equals(alloc, existing->name.begin, existing->name.end,
                 name.begin, name.end);
        if (same) {
            return check;
        }
    }

    return 0;
}

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
                if (sa_equals(alloc, cand->name.begin, cand->name.end, d->begin, d->end)) {
                    /* Ensure uniqueness: check if cand is already in list by name */
                    target** duplicate = find_last_target_with_same_name(list_begin, list_end, cand->name, alloc);
                    if (!duplicate) {
                        target** push = sa_alloc(alloc, sizeof(*push));
                        *push = cand;
                        list_end = alloc->cursor;
                    }

                    /* Found matching target, no need to check other candidates */
                    break;
                }
            }

            d = (void*)d->end;
        }
    }

    return list_begin;
}

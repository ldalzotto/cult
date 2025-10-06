#include "primitive.h"
#include "mem.h"
#include "agent_args.h"

arguments extract_arguments(i32 argc, char** argv) {
    arguments out;
    out.file_path.begin = 0; out.file_path.end = 0;
    out.api_key.begin = 0; out.api_key.end = 0;
    out.model.begin = 0; out.model.end = 0;

    if (argc < 2) {
        return out;
    }

    // file path is first positional argument
    out.file_path.begin = (u8*)argv[1];
    out.file_path.end = (u8*)argv[1] + mem_cstrlen(argv[1]);

    // parse --key
    for (i32 i = 1; i < argc; ++i) {
        const char* arg = (const char*)argv[i];
        // match "--key"
        if (arg[0] == '-' && arg[1] == '-' &&
            arg[2] == 'k' && arg[3] == 'e' && arg[4] == 'y' && arg[5] == '\0') {
            if (i + 1 < argc) {
                const char* val = argv[i + 1];
                out.api_key.begin = (u8*)val;
                out.api_key.end = (u8*)val + mem_cstrlen((void*)val);
                i++;
                break;
            }
        }
    }

    // parse --model
    for (i32 i = 1; i < argc; ++i) {
        const char* arg = (const char*)argv[i];
        // match "--model"
        if (arg[0] == '-' && arg[1] == '-' &&
            arg[2] == 'm' && arg[3] == 'o' && arg[4] == 'd' && arg[5] == 'e' &&
            arg[6] == 'l' && arg[7] == '\0') {
            if (i + 1 < argc) {
                const char* val = argv[i + 1];
                out.model.begin = (u8*)val;
                out.model.end = (u8*)val + mem_cstrlen((void*)val);
                i++;
                break;
            }
        }
    }

    return out;
}

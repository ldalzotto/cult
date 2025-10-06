#ifndef TOOLS_AGENT_AGENT_ARGS_H
#define TOOLS_AGENT_AGENT_ARGS_H

#include "primitive.h"
#include "litteral.h"

typedef struct {
    string file_path;
    string api_key;
    string model;
} arguments;

arguments extract_arguments(i32 argc, char** argv);

#endif // TOOLS_AGENT_AGENT_ARGS_H

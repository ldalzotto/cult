#include "huffman.h"

#include "hf_frequency.h"
#include "hf_tree_build.h"
#include "hf_code_table_build.h"
#include "hf_serialize.h"
#include "hf_deserialize.h"

void* huffman_compress(u8* begin, u8* end, stack_alloc* alloc) {
    void* out = alloc->cursor;
    hf_frequency_array frequency = hf_frequency(begin, end, alloc);
    hf_tree tree = hf_tree_build(frequency, alloc);
    hf_code_table table = hf_code_table_build(&tree, alloc);
    void* serialized = hf_serialize(begin, end, tree, &table, alloc);
    sa_move_tail(alloc, serialized, out);
    return out;
}

void* huffman_decompress(u8* begin, u8* end, stack_alloc* alloc) {
    return hf_deserialize(begin, end, alloc);
}

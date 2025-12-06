#include "huffman.h"

#include "hf_frequency.h"
#include "hf_tree_build.h"

void* huffman_compress(u8* begin, u8* end, stack_alloc* alloc) {
    void* out = alloc->cursor;
    hf_frequency_array frequency = hf_frequency(begin, end, alloc);
    hf_tree tree = hf_tree_build(frequency, alloc);
    unused(tree);
    
    // TODO: serialize tree

    sa_free(alloc, out);
    return out;
}

/**
 * @file lzss.h
 * @brief LZSS (Lempel-Ziv-Storer-Szymanski) compression algorithm implementation.
 *
 * This header provides functions for compressing and decompressing data using the LZSS algorithm,
 * which is a dictionary-based compression technique that replaces repeated sequences of data
 * with references to previously seen data.
 *
 * The implementation uses a sliding window approach with configurable parameters for match sizes
 * and window size to balance compression ratio and performance.
 */

#ifndef LZSS_H
#define LZSS_H

#include "stack_alloc.h"
#include "file.h"
#include "lzss_config.h"

/**
 * @brief Compresses data using the LZSS algorithm.
 *
 * This function takes a range of data and compresses it using the LZSS algorithm with the provided
 * configuration. It uses a stack allocator for memory management and can output debug information
 * to a file if specified.
 *
 * @param begin Pointer to the start of the data to compress.
 * @param end Pointer to the end of the data to compress.
 * @param config Configuration parameters for the compression.
 * @param alloc Stack allocator for managing memory during compression.
 * @param debug File handle for debug output (can be NULL to disable debug output).
 * @return Pointer to the compressed data (allocated via the stack allocator).
 */
void* lzss_compress(u8* begin, u8* end, lzss_config config, stack_alloc* alloc, file_t debug);

/**
 * @brief Decompresses data compressed with LZSS algorithm.
 *
 * This function decompresses LZSS-compressed data back to its original form. It uses a stack
 * allocator for memory management and can output debug information to a file if specified.
 *
 * @param begin Pointer to the start of the compressed data.
 * @param end Pointer to the end of the compressed data.
 * @param alloc Stack allocator for managing memory during decompression.
 * @param debug File handle for debug output (can be NULL to disable debug output).
 * @return Pointer to the decompressed data (allocated via the stack allocator).
 */
void* lzss_decompress(u8* begin, u8* end, stack_alloc* alloc, file_t debug);

#endif

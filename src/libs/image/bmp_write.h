#ifndef BMP_WRITE_H
#define BMP_WRITE_H

#include "stack_alloc.h"
#include "bmp_read.h"

/*
 bmp_write: write a bmp_file (as returned by bmp_read) into a BMP file buffer.

 Parameters:
 - in: source image. Expected to be in the same format returned by bmp_read:
       width, height, channels==4, pixels pointer (top-to-bottom, RGBA).
 - write_32bpp: if non-zero, write 32bpp BMP (preserving alpha channel).
                if zero, write 24bpp BMP (drops alpha, rows padded to 4-byte boundary).
 - alloc: stack allocator used to allocate the resulting BMP buffer.

 Returns:
 - struct with data pointer (allocated from alloc) and size in bytes.
   On failure, data == NULL and size == 0.
*/

u8* bmp_write(bmp_file in, int write_32bpp, stack_alloc* alloc);

#endif /* BMP_WRITE_H */

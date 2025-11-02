#ifndef BMP_READ_H
#define BMP_READ_H

#include "stack_alloc.h"

/*
 Public BMP information returned by bmp_read:
 - width, height: image dimensions in pixels
 - channels: always 4 (RGBA) for the current implementation
 - pixels: pointer to pixel data allocated from the provided stack_alloc.
           Pixel layout is row-major, top-to-bottom, each pixel is 4 bytes:
           R, G, B, A (one byte each).
 Note: Only uncompressed BMP (BI_RGB) with 24bpp or 32bpp input is supported.
       The returned pixel buffer is always 4 bytes per pixel (RGBA).
*/
typedef struct {
    u32 width;
    u32 height;
    u8 channels;   // number of bytes per pixel in the returned buffer (always 4)
    u8* pixels;    // pointer into stack_alloc memory; NULL on failure
} bmp_file;

bmp_file bmp_read(u8* begin, u8* end, stack_alloc* alloc);

#endif /*BMP_READ_H*/

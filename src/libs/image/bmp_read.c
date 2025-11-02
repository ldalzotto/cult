#include "./bmp_read.h"

/* Helper to read little-endian integers safely */
static u32 read_u32_le(u8* p, u8* end) {
    if ((uptr)(end - p) < 4) return 0;
    return (u32)p[0] | ((u32)p[1] << 8) | ((u32)p[2] << 16) | ((u32)p[3] << 24);
}
static u16 read_u16_le(u8* p, u8* end) {
    if ((uptr)(end - p) < 2) return 0;
    return (u16)p[0] | ((u16)p[1] << 8);
}

/* Only support raw (BI_RGB) 24bpp and 32bpp BMP files.
   Returned pixel buffer is always 4 bytes per pixel in RGBA order.
*/
bmp_file bmp_read(u8* begin, u8* end, stack_alloc* alloc) {
    bmp_file out = {0, 0, 0, 0};

    if (!begin || !end || begin >= end) return out;

    const uptr BITMAPINFOHEADER = 40;
    const uptr BITMAPFILEHEADER = 14;
    uptr total_size = (uptr)(end - begin);

    const uptr MINIMUM_HEADER_SIZE = BITMAPFILEHEADER + BITMAPINFOHEADER;
    if (total_size < MINIMUM_HEADER_SIZE) return out;

    // BMP signature
    if (begin[0] != 'B' || begin[1] != 'M') return out;

    // Pixel data offset at file header offset 10
    u8* p = begin;
    u32 pixel_data_offset = read_u32_le(p + 10, end);
    if (pixel_data_offset == 0) return out;
    if ((uptr)pixel_data_offset > total_size) return out;

    // DIB header
    u32 dib_size = read_u32_le(p + BITMAPFILEHEADER, end);
    if (dib_size < BITMAPINFOHEADER) return out; // require BITMAPINFOHEADER or larger

    u32 width = read_u32_le(p + 18, end);
    // height can be signed: negative indicates top-down bitmap
    i32 height_signed = (i32)read_u32_le(p + 22, end);
    u32 abs_height = height_signed < 0 ? (u32)(-height_signed) : (u32)height_signed;
    u16 planes = read_u16_le(p + 26, end);
    if (planes != 1) return out;

    u16 bpp = read_u16_le(p + 28, end);
    u32 compression = read_u32_le(p + 30, end);
    if (compression != 0) return out; // only BI_RGB supported

    if (bpp != 24 && bpp != 32) return out; // only support 24bpp and 32bpp inputs

    // Validate pixel data area fits inside input buffer
    u8* pixel_data = begin + pixel_data_offset;
    if (pixel_data < begin || pixel_data > end) return out;

    // Compute source row size (padded to 4 bytes for 24bpp)
    uptr src_row_size;
    if (bpp == 24) {
        uptr row_bytes = (uptr)width * 3;
        // rows are padded to 4-byte boundary
        src_row_size = (row_bytes + 3) & ~((uptr)3);
    } else { // 32 bpp
        src_row_size = (uptr)width * 4;
    }

    // Ensure whole pixel array fits in provided buffer (best-effort check)
    uptr required_src = src_row_size * abs_height;
    if ((uptr)(end - pixel_data) < required_src) return out;

    // Allocate destination buffer: width * abs_height * 4 bytes (RGBA)
    uptr dst_stride = (uptr)width * 4;
    uptr dst_size = dst_stride * abs_height;
    u8* dst = sa_alloc(alloc, dst_size);
    if (!dst) return out;

    // Copy/convert rows. BMP stores pixels in B,G,R,(A) order.
    int top_down = (height_signed < 0) ? 1 : 0;
    for (u32 y = 0; y < abs_height; ++y) {
        uptr src_row_index = top_down ? y : (abs_height - 1 - y);
        u8* src_row = pixel_data + src_row_index * src_row_size;
        u8* dst_row = dst + (uptr)y * dst_stride;

        for (u32 x = 0; x < width; ++x) {
            if (bpp == 24) {
                u8 b = src_row[x * 3 + 0];
                u8 g = src_row[x * 3 + 1];
                u8 r = src_row[x * 3 + 2];
                dst_row[x * 4 + 0] = r;
                dst_row[x * 4 + 1] = g;
                dst_row[x * 4 + 2] = b;
                dst_row[x * 4 + 3] = 255;
            } else { // 32 bpp
                u8 b = src_row[x * 4 + 0];
                u8 g = src_row[x * 4 + 1];
                u8 r = src_row[x * 4 + 2];
                u8 a = src_row[x * 4 + 3];
                dst_row[x * 4 + 0] = r;
                dst_row[x * 4 + 1] = g;
                dst_row[x * 4 + 2] = b;
                dst_row[x * 4 + 3] = a;
            }
        }
    }

    out.width = width;
    out.height = abs_height;
    out.channels = 4;
    out.pixels = dst;
    return out;
}

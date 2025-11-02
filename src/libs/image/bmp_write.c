#include "./bmp_write.h"

/* Helpers to write little-endian integers */
static void write_u16_le(u8* p, u16 v) {
    p[0] = (u8)(v & 0xFF);
    p[1] = (u8)((v >> 8) & 0xFF);
}
static void write_u32_le(u8* p, u32 v) {
    p[0] = (u8)(v & 0xFF);
    p[1] = (u8)((v >> 8) & 0xFF);
    p[2] = (u8)((v >> 16) & 0xFF);
    p[3] = (u8)((v >> 24) & 0xFF);
}
static void write_i32_le(u8* p, i32 v) {
    write_u32_le(p, (u32)v);
}

u8* bmp_write(bmp_file in, int write_32bpp, stack_alloc* alloc) {
    if (!in.pixels) return alloc->cursor;
    if (in.width == 0 || in.height == 0) return alloc->cursor;
    if (in.channels == 0) return alloc->cursor;

    const uptr BITMAPFILEHEADER = 14;
    const uptr BITMAPINFOHEADER = 40;

    // Choose bpp for output
    u16 bpp = write_32bpp ? 32 : 24;

    // Compute row sizes
    uptr row_bytes_unpadded = (uptr)in.width * (bpp / 8);
    uptr row_stride;
    if (bpp == 24) {
        // rows are padded to 4-byte boundary
        row_stride = (row_bytes_unpadded + 3) & ~((uptr)3);
    } else {
        row_stride = row_bytes_unpadded;
    }

    uptr pixel_data_size = row_stride * (uptr)in.height;
    uptr file_size = BITMAPFILEHEADER + BITMAPINFOHEADER + pixel_data_size;
    // Allocate buffer for file
    u8* buf = sa_alloc(alloc, file_size);

    // Start writing
    // BITMAPFILEHEADER (14 bytes)
    buf[0] = 'B';
    buf[1] = 'M';
    write_u32_le(buf + 2, (u32)file_size); // file size
    write_u32_le(buf + 6, 0); // reserved (4 bytes, two u16s)
    write_u32_le(buf + 10, (u32)(BITMAPFILEHEADER + BITMAPINFOHEADER)); // pixel data offset

    // BITMAPINFOHEADER (40 bytes)
    u8* dib = buf + BITMAPFILEHEADER;
    write_u32_le(dib + 0, (u32)BITMAPINFOHEADER); // header size
    write_u32_le(dib + 4, in.width);               // width (unsigned)
    // Use negative height to indicate top-down bitmap so that row 0 is top
    write_i32_le(dib + 8, -((i32)in.height));     // height (signed, negative = top-down)
    write_u16_le(dib + 12, 1);                     // planes
    write_u16_le(dib + 14, bpp);                   // bit count
    write_u32_le(dib + 16, 0);                     // compression = BI_RGB
    write_u32_le(dib + 20, (u32)pixel_data_size);  // image size (can be 0 for BI_RGB, but set it)
    write_u32_le(dib + 24, 0);                     // x pixels per meter
    write_u32_le(dib + 28, 0);                     // y pixels per meter
    write_u32_le(dib + 32, 0);                     // colors used
    write_u32_le(dib + 36, 0);                     // important colors

    // Pixel data starts here
    u8* pixel_data = buf + BITMAPFILEHEADER + BITMAPINFOHEADER;

    // Input pixels are expected as top-to-bottom, row-major, RGBA (4 bytes per pixel)
    uptr in_stride = (uptr)in.width * (uptr)in.channels;

    for (u32 y = 0; y < in.height; ++y) {
        u8* dst_row = pixel_data + (uptr)y * row_stride;
        u8* src_row = in.pixels + (uptr)y * in_stride;

        if (bpp == 32) {
            // Write B, G, R, A
            for (u32 x = 0; x < in.width; ++x) {
                u8 r = src_row[x * in.channels + 0];
                u8 g = src_row[x * in.channels + 1];
                u8 b = src_row[x * in.channels + 2];
                u8 a = (in.channels >= 4) ? src_row[x * in.channels + 3] : 255;
                dst_row[x * 4 + 0] = b;
                dst_row[x * 4 + 1] = g;
                dst_row[x * 4 + 2] = r;
                dst_row[x * 4 + 3] = a;
            }
        } else { // 24bpp
            for (u32 x = 0; x < in.width; ++x) {
                u8 r = src_row[x * in.channels + 0];
                u8 g = src_row[x * in.channels + 1];
                u8 b = src_row[x * in.channels + 2];
                dst_row[x * 3 + 0] = b;
                dst_row[x * 3 + 1] = g;
                dst_row[x * 3 + 2] = r;
            }
            // padding zeros
            uptr row_bytes = in.width * 3;
            uptr pad = row_stride - row_bytes;
            if (pad) {
                for (uptr i = 0; i < pad; ++i) dst_row[row_bytes + i] = 0;
            }
        }
    }

    return buf;
}

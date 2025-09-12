
#include <X11/Xlib.h>
#include "../mem.h"
#include "../stack_alloc.h"
#include "../assert.h"

typedef struct {
    stack_alloc alloc;
    void* display;
    void* images;
    void* windows;
    void* end;
} memory;

static memory memory_init(void) {
    uptr size = 2048 * 2048;
    void* pointer = mem_map(size);
    stack_alloc alloc;
    sa_init(&alloc, pointer, byteoffset(pointer, size));
    memory m;
    m.alloc = alloc;
    m.display = alloc.cursor;
    m.images = alloc.cursor;
    m.windows = alloc.cursor;
    m.end = alloc.end;
    return m;
}

typedef struct x11_stub_window x11_stub_window;
static void window_offset(x11_stub_window* self, iptr offset);
static void* window_end(x11_stub_window* self);
static uptr window_size(void);
static void window_set_buffer(x11_stub_window* self, void* begin, void* end);

static XImage* memory_alloc_image(memory* self) {
    static const uptr image_size = sizeof(XImage);
    void* image_allocated = self->windows;
    // move windows
    
    sa_alloc(&self->alloc, image_size);
    sa_move(&self->alloc, 
            self->windows, byteoffset(self->windows, image_size), 
            bytesize(self->windows, self->end));
    self->windows = byteoffset(self->windows, image_size);
    self->end = byteoffset(self->windows, image_size);

    do {
        void* window = self->windows;
        while (window < self->end) {
            window_offset(window, image_size);
            window = window_end(window);
        }
    } while(0);

    return image_allocated;
}

static void memory_free_image(memory* self, XImage* image) {
    void* from = byteoffset(image, sizeof(*image));
    void* to = image;
    uptr size = bytesize(from, self->end);
    sa_move(&self->alloc, from, to, size);
    
    self->images = byteoffset(self->images, -bytesize(from, to))
    byteoffset(self->end, -sizeof(*image));
}

static x11_stub_window* memory_alloc_window(memory* self) {
    void* window = sa_alloc(&self->alloc, window_size());
    self->end = byteoffset(self->end, window_size());
    window_set_buffer(window, self->alloc.cursor, self->alloc.cursor);
    return window;
}

static void memory_windows_offset(x11_stub_window* start, x11_stub_window* end, iptr offset) {
    do {
        void* window = start;
        while (window < (void*)end) {
            window_offset(window, offset);
            window = window_end(window);
        }
    } while(0);
}

static void memory_alloc_window_buffer(memory* self, x11_stub_window* window, uptr size) {
    debug_assert(self->alloc.cursor == self->end);
    void* buffer = sa_alloc(&self->alloc, size);
    window_set_buffer(window, buffer, self->alloc.cursor);
    self->end = byteoffset(self->end, size);
}

struct x11_stub_window {
    unsigned int width;
    unsigned int height;
    void* name;
    long event_mask;
    void* buffer_begin;
    void* buffer_end;
};

static void window_offset(x11_stub_window* self, iptr offset) {
    self->buffer_begin = byteoffset(self->buffer_begin, offset);
    self->buffer_end = byteoffset(self->buffer_end, offset);
}

static void* window_end(x11_stub_window* self) {
    return self->buffer_end;
}

static void window_set_buffer(x11_stub_window* self, void* begin, void* end) {
    self->buffer_begin = begin;
    self->buffer_end = end;
}

typedef struct x11_stub_display {
    stack_alloc alloc;
    _XPrivDisplay priv;
    XImage* images;
    x11_stub_window* window;
} x11_stub_display;

extern Display *XOpenDisplay(_Xconst char* display_name) {
    uptr size = 2048 * 2048;
    void* p = mem_map(size);
    stack_alloc alloc;
    sa_init(&alloc, p, byteoffset(p, size));

    x11_stub_display* stub_display = sa_alloc(&alloc, sizeof(*stub_display));
    _XPrivDisplay display = sa_alloc(&alloc, sizeof(*display));
    display->private1 = (void*)stub_display;
    display->display_name = (char*)display_name;
    display->default_screen = 0;

    Screen* screen = sa_alloc(&alloc, sizeof(*screen));
    screen->width = 32;
    screen->height = 32;
    display->screens = screen;

    stub_display->alloc = alloc;

    stub_display->images = alloc.cursor;
    stub_display->window = alloc.cursor;
    return (Display *)display;
}

 Window XCreateSimpleWindow(
    Display* _display,
    Window parent,
    int x,
    int y,
    unsigned int width,
    unsigned int height,
    unsigned int border_width,
    unsigned long border,
    unsigned long background
) {
    x11_stub_display* display = (x11_stub_display*)((_XPrivDisplay)_display)->private1;
    x11_stub_window* window = alloc_window(display);
    alloc_window_buffer(window, width * height * 4, &display->alloc);
    window->width = width;
    window->height = height;
    return (Window)window;
}

int XDestroyWindow(
    Display* _display,
    Window _w
) {
    x11_stub_display* self = (x11_stub_display*)((_XPrivDisplay)_display)->private1;
    x11_stub_window* window = (x11_stub_window*)_w;

    x11_stub_window* window_previous = self->window;
    do {
        x11_stub_window* w = self->window;
        while (w < self->alloc.c) {
        
        }
    }while (1);
    sa_move_tail(&self->alloc, void *from, void *to);
}

int XStoreName(
    Display* _display,
    Window	_w,
    _Xconst char* window_name
) {
    x11_stub_display* display = (x11_stub_display*)((_XPrivDisplay)_display)->private1;
    x11_stub_window* window = (x11_stub_window*)_w;
    window->name = (void*)window_name;
    return 0;
}

int XSelectInput(
    Display* _display,
    Window _w,
    long event_mask
) {

    x11_stub_display* display = (x11_stub_display*)((_XPrivDisplay)_display)->private1;
    x11_stub_window* window = (x11_stub_window*)_w;
    window->event_mask = event_mask;
    return 0;
}

int XMapWindow(
    Display* _display,
    Window _w
) {
    x11_stub_display* display = (x11_stub_display*)((_XPrivDisplay)_display)->private1;
    x11_stub_window* window = (x11_stub_window*)_w;
    display->window = window;
    return 0;
}

XImage *XCreateImage(
    Display* _display,
    Visual*	visual,
    unsigned int	depth,
    int	format,
    int offset,
    char* data,
    unsigned int width,
    unsigned int height,
    int	bitmap_pad,
    int	bytes_per_line
) {
    x11_stub_display* display = (x11_stub_display*)((_XPrivDisplay)_display)->private1;
    XImage* image = alloc_image(display);
    image->data = data;
    image->width = width;
    image->height = height;
    image->bitmap_pad = bitmap_pad;
    image->bytes_per_line = bytes_per_line;
    image->format = format;
    return image;
}


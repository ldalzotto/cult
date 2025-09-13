
#include <X11/Xlib.h>
#include "../mem.h"
#include "../stack_alloc.h"
#include "../assert.h"

typedef struct {
    stack_alloc alloc;
    void* display;
    void* screen;
    void* image;
    void* window;
    void* end;
} memory;

static void memory_init(memory* m) {
    uptr size = 2048 * 2048;
    void* pointer = mem_map(size);
    stack_alloc alloc;
    sa_init(&alloc, pointer, byteoffset(pointer, size));
    m->alloc = alloc;
    m->display = alloc.cursor;
    m->screen = alloc.cursor;
    m->image = alloc.cursor;
    m->window = alloc.cursor;
    m->end = alloc.cursor;
}

static void memory_deinit(memory* m) {
    debug_assert(m->alloc.cursor == m->end);
    mem_unmap(m->alloc.begin, bytesize(m->alloc.begin, m->alloc.end));
    sa_deinit(&m->alloc);
}

static void memory_sanitize(memory* m) {
    debug_assert(m->alloc.cursor == m->end);
    debug_assert(m->display <= m->screen);
    debug_assert(m->screen <= m->image);
    debug_assert(m->image <= m->window);
    debug_assert(m->window <= m->end);
}

static uptr x11_stub_display_size(void);

static _XPrivDisplay memory_alloc_display(memory* self) {
    debug_assert(self->end == self->alloc.cursor);
    uptr offset = x11_stub_display_size();
    self->display = sa_alloc(&self->alloc, offset);
    self->screen = byteoffset(self->image, offset);
    self->image = byteoffset(self->image, offset);
    self->window = byteoffset(self->window, offset);
    self->end = byteoffset(self->end, offset);
    return self->display;
}

static void memory_free_display(memory* self) {
    debug_assert(self->alloc.cursor == self->screen);
    debug_assert(self->alloc.cursor == self->image);
    debug_assert(self->alloc.cursor == self->window);
    debug_assert(self->alloc.cursor == self->end);
    
    sa_free(&self->alloc, self->display);
    self->screen = self->display;
    self->image = self->display;
    self->window = self->display;
    self->end = self->display;
}

static void* memory_alloc_screen(memory* self) {
    debug_assert(self->screen == self->alloc.cursor);
    static const uptr screen_size = sizeof(Screen);
    self->screen = sa_alloc(&self->alloc, screen_size);
    self->image = byteoffset(self->image, screen_size);
    self->window = byteoffset(self->window, screen_size);
    self->end = byteoffset(self->end, screen_size);
    return self->screen;
}

static void memory_free_screen(memory* self) {
    debug_assert(self->alloc.cursor == self->image);
    debug_assert(self->alloc.cursor == self->window);
    debug_assert(self->alloc.cursor == self->end);
    
    sa_free(&self->alloc, self->screen);
    self->image = self->screen;
    self->window = self->screen;
    self->end = self->screen;
}

typedef struct x11_stub_window x11_stub_window;
static void window_offset(x11_stub_window* self, iptr offset);
static void* window_end(x11_stub_window* self);
static uptr window_size(void);
static void window_set_buffer(x11_stub_window* self, void* begin, void* end);

static const uptr image_size = sizeof(XImage);
static XImage* memory_alloc_image(memory* self) {
    // move window
    sa_alloc(&self->alloc, image_size);
    uptr move_size = bytesize(self->window, self->end);
    sa_move(&self->alloc, 
            self->window, byteoffset(self->window, image_size), 
            move_size);
    self->window = byteoffset(self->window, image_size);
    
    if (self->window < self->end) {
        window_offset(self->window, image_size);
        self->end = window_end(self->window);
    } else {
        self->end = byteoffset(self->end, image_size);
    }

    return self->image;
}

static void memory_free_image(memory* self, XImage* image) {
    void* to = image;
    void* from = byteoffset(to, image_size);
    sa_move(&self->alloc, from, to, bytesize(from, self->end));
    
    self->window = byteoffset(self->window, -image_size);
    self->end = byteoffset(self->end, -image_size);
    sa_free(&self->alloc, self->end);
    
    if (self->window < self->end) {
        window_offset(self->window, -image_size);
    }
}

static x11_stub_window* memory_alloc_window(memory* self) {
    debug_assert(self->alloc.cursor == self->window);
    void* window = sa_alloc(&self->alloc, window_size());
    self->end = byteoffset(self->end, window_size());
    window_set_buffer(window, self->alloc.cursor, self->alloc.cursor);
    return window;
}

static void memory_alloc_window_buffer(memory* self, x11_stub_window* window, uptr size) {
    debug_assert(self->alloc.cursor == self->end);
    void* buffer = sa_alloc(&self->alloc, size);
    window_set_buffer(window, buffer, self->alloc.cursor);
    self->end = byteoffset(self->end, size);
}

static void memory_free_window(memory* self, x11_stub_window* window) {
    void* from = window_end(window);
    void* to = window;
    sa_move(&self->alloc, from, to, bytesize(from, self->end));

    iptr offset = -bytesize(to, from);
    self->end = byteoffset(self->end, offset);
    sa_free(&self->alloc, self->end);
}


//////////////////

static uptr x11_stub_display_size(void) {
    static const _XPrivDisplay tmp;
    return sizeof(*tmp);
}

struct x11_stub_window {
    unsigned int width;
    unsigned int height;
    void* name;
    long event_mask;
    void* buffer_begin;
    void* buffer_end;
};

static uptr window_size(void) {
    return sizeof(x11_stub_window);
}

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

static memory* display_get_memory(Display* d) {
    _XPrivDisplay p = (_XPrivDisplay)d;
    return (memory*)p->private1;
}

static void display_set_memory(Display* d, memory* m) {
    _XPrivDisplay p = (_XPrivDisplay)d;
    p->private1 = (void*)m;
}

static x11_stub_window* display_get_window(Display* d) {
    _XPrivDisplay p = (_XPrivDisplay)d;
    return (x11_stub_window*)p->private11;
}

static void display_set_window(Display* d, x11_stub_window* window) {
    _XPrivDisplay p = (_XPrivDisplay)d;
    p->private11 = (void*)window;
}

static void display_sanitize(Display* d) {
    memory* m = display_get_memory(d);
    memory_sanitize(m);
    x11_stub_window* window = display_get_window(d);
    debug_assert(window == m->window);
    debug_assert((void*)window <= m->end);
    if ((void*)window != m->end) {
        debug_assert(window->buffer_begin > m->window);
        debug_assert(window->buffer_begin <= window->buffer_end);
        debug_assert(window->buffer_end == m->end);
    }
}

#if !DEBUG_ASSERTIONS_ENABLED
#define display_sanitize(...)
#endif

Display *XOpenDisplay(_Xconst char* display_name) {
    uptr size = 2048 * 2048;
    void* p = mem_map(size);
    stack_alloc alloc;
    sa_init(&alloc, p, byteoffset(p, size));

    memory* m = sa_alloc(&alloc, sizeof(*m));
    memory_init(m);
    _XPrivDisplay stub_display = memory_alloc_display(m);
    stub_display->display_name = (char*)display_name;
    stub_display->default_screen = 0;
    Screen* screen = memory_alloc_screen(m);
    stub_display->screens = screen;

    screen->width = 32;
    screen->height = 32;

    Display* display = (Display *)stub_display;
    display_set_memory(display, m);
    display_set_window(display, m->window);

    display_sanitize(display);
    return display;
}

int XCloseDisplay(
    Display* display
) {
    display_sanitize(display);
    memory* m = display_get_memory(display);
    memory_free_screen(m);
    memory_free_display(m);
    memory_deinit(m);
    return 0;
}

int XFlush(
    Display* display
) {
    display_sanitize(display);
    return 0;
}

Window XCreateSimpleWindow(
    Display* display,
    Window parent,
    int x,
    int y,
    unsigned int width,
    unsigned int height,
    unsigned int border_width,
    unsigned long border,
    unsigned long background
) {
    display_sanitize(display);

    unused(parent);
    unused(x);
    unused(y);
    unused(border_width);
    unused(border);
    unused(background);

    memory* m = display_get_memory(display);
    x11_stub_window* window = memory_alloc_window(m);
    window->width = width;
    window->height = height;
    memory_alloc_window_buffer(m, window, width * height * 4);
    display_sanitize(display);
    return (Window)window;
}

int XDestroyWindow(
    Display* display,
    Window _w
) {
    display_sanitize(display);
    memory* m = display_get_memory(display);
    x11_stub_window* w = (x11_stub_window*)_w;
    memory_free_window(m, w);
    display_sanitize(display);
    return 0;
}

int XStoreName(
    Display* display,
    Window	_w,
    _Xconst char* window_name
) {
    display_sanitize(display);
    x11_stub_window* window = (x11_stub_window*)_w;
    window->name = (void*)window_name;
    return 0;
}

int XSelectInput(
    Display* display,
    Window _w,
    long event_mask
) {
    display_sanitize(display);
    x11_stub_window* window = (x11_stub_window*)_w;
    window->event_mask = event_mask;
    return 0;
}

int XMapWindow(
    Display* display,
    Window w
) {
    display_sanitize(display);
    display_set_window(display, (x11_stub_window*)w);
    display_sanitize(display);
    return 0;
}

static int DestroyImage(XImage* image) {
    memory* m = (memory*)image->obdata;
    display_sanitize(m->display);
    void* window_before = m->window;
    memory_free_image(m, image);
    if (window_before != m->window) {
        display_set_window(m->display, m->window);
    }
    display_sanitize(m->display);
    return 0;
}

XImage *XCreateImage(
    Display* display,
    Visual*	visual,
    unsigned int depth,
    int	format,
    int offset,
    char* data,
    unsigned int width,
    unsigned int height,
    int	bitmap_pad,
    int	bytes_per_line
) {
    display_sanitize(display);

    unused(visual);
    unused(depth);
    unused(offset);

    memory* m = display_get_memory(display);
    void* window_before = m->window;
    XImage* image = memory_alloc_image(m);
    if (window_before != m->window) {
        display_set_window(display, m->window);
    }
    display_sanitize(display);
    image->data = data;
    image->width = width;
    image->height = height;
    image->bitmap_pad = bitmap_pad;
    image->bytes_per_line = bytes_per_line;
    image->format = format;
    image->obdata = (void*)m;
    image->f.destroy_image = DestroyImage;
    return image;
}

int XPutImage(
    Display* display,
    Drawable d,
    GC gc,
    XImage* image,
    int src_x,
    int src_y,
    int dest_x,
    int dest_y,
    unsigned int width,
    unsigned int height
) {
    display_sanitize(display);
    
    unused(d);
    unused(gc);
    unused(src_x);
    unused(src_y);
    unused(dest_x);
    unused(dest_y);

    x11_stub_window* window = display_get_window(display);
    uptr copy_size = width * height * 4;
    debug_assert(byteoffset(window->buffer_begin, copy_size) == window->buffer_end);
    __builtin_memcpy(
        window->buffer_begin,
        image->data,
        copy_size
    );
    display_sanitize(display);
    return 0;
}

int XPending(
    Display* display
) {
    display_sanitize(display);
    return 0;
}

int XNextEvent(
    Display* display,
    XEvent*	event_return
) {
    display_sanitize(display);
    unused(event_return);
    return 0;
}

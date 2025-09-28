#ifndef WIN_H
#define WIN_H

typedef enum { WIN_EVENT_TYPE_PRESSED, WIN_EVENT_TYPE_RELEASED, WIN_EVENT_TYPE_UNKNOWN } win_event_type;
typedef enum { WIN_KEY_UP, WIN_KEY_DOWN, WIN_KEY_LEFT, WIN_KEY_RIGHT, WIN_KEY_UNKNOWN } win_key;

/**
 * @brief Simple event structure
 */
typedef struct {
    win_event_type type;
    win_key key;
} win_event;

typedef struct {
    void* begin;
    void* end;
} win_buffer;

#endif /* WIN_H */

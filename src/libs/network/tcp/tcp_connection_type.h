#ifndef TCP_CONNECTION_TYPE_H
#define TCP_CONNECTION_TYPE_H

#include "file.h"

struct tcp {
    file_t fd;
    struct addrinfo* res;
    struct addrinfo* chosen;
};

#endif /* TCP_CONNECTION_TYPE_H */

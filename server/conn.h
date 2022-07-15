#ifndef CONN_H
#define CONN_H

#include "socket_include.h"
#include <stdbool.h>

struct conn
{
    struct sockaddr_in addr;
    int sockfd;
};

bool conn_listen(struct conn *conn);

#endif

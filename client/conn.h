#ifndef CONN_H
#define CONN_H

#include "addr.h"
#include <stdbool.h>

enum conn_state
{
    CONN_INIT,
    CONN_PEND,
    CONN_RUN,
    CONN_FAIL,
    CONN_DONE,
};

struct conn
{
    enum conn_state state;
    struct sockaddr_in addr;
    int sockfd;
};

bool conn_init(struct conn *conn);
bool conn_start_connecting(struct conn *conn);
void conn_check_connection(struct conn *conn);
void conn_close(struct conn *conn);

#endif

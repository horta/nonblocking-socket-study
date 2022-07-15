#ifndef CLIENT_H
#define CLIENT_H

#include "conn.h"
#include "server_config.h"
#include <ev.h>
#include <stdbool.h>

struct client;

void clients_init(void);
struct client *clients_next_avail(void);

struct conn *client_conn(struct client *client);
bool client_accept(struct client *cl, int server_sockfd);

#endif

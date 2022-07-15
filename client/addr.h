#ifndef ADDR_H
#define ADDR_H

#include "socket_include.h"

enum addr_ipv
{
    IPV4,
    IPV6
};

int addr_setup(struct sockaddr_in *addr, enum addr_ipv ipv, char const *ip,
               uint16_t port);

#endif

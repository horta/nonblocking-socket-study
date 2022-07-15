#include "addr.h"

int addr_setup(struct sockaddr_in *addr, enum addr_ipv ipv, char const *ip,
               uint16_t port)
{
    addr->sin_family = ipv == IPV4 ? AF_INET : AF_INET6;
    addr->sin_port = htons(port);
    if ((inet_pton(addr->sin_family, ip, &addr->sin_addr)) != 1) return -1;
    return 0;
}

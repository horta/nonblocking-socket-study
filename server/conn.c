#include "conn.h"
#include "report.h"
#include "server_config.h"
#include <fcntl.h>
#include <stdbool.h>

enum addr_ipv
{
    IPV4,
    IPV6
};

static void setup_addr(struct sockaddr_in *addr, enum addr_ipv ipv,
                       uint16_t port)
{
    addr->sin_family = ipv == IPV4 ? AF_INET : AF_INET6;
    addr->sin_port = htons(port);
    addr->sin_addr.s_addr = htonl(INADDR_ANY);
}

static inline bool set_socket_nonblocking(int sockfd)
{
    return fcntl(sockfd, F_SETFL, fcntl(sockfd, F_GETFL, 0) | O_NONBLOCK) != -1;
}

static inline bool set_socket_reuseaddr(int sockfd)
{
    int const y = 1;
    return setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &y, sizeof(y)) != -1;
}

static inline bool set_socket_linger(int sockfd, int seconds)
{
    struct linger linger = {.l_onoff = 1, .l_linger = seconds};
    socklen_t size = sizeof(linger);
    return setsockopt(sockfd, SOL_SOCKET, SO_LINGER, &linger, size) != -1;
}

static bool bind_and_listen(struct conn *conn)
{
    socklen_t size = sizeof(conn->addr);
    int const sockfd = conn->sockfd;
    if (bind(sockfd, (struct sockaddr const *)&conn->addr, size) == -1)
    {
        echo("failed to bind");
        return false;
    }

    if (listen(sockfd, BACKLOG) == -1)
    {
        echo("failed to listen");
        return false;
    }
    return true;
}

bool conn_listen(struct conn *conn)
{
    setup_addr(&conn->addr, IPV4, PORT);

    int const sockfd = socket(conn->addr.sin_family, SOCK_STREAM, IPPROTO_TCP);
    if (sockfd == -1)
    {
        echo("failed to create socket");
        return false;
    }

    if (!set_socket_reuseaddr(sockfd))
    {
        echo("failed to set socket option");
        goto cleanup;
    }

    if (!set_socket_nonblocking(sockfd))
    {
        echo("failed to set socket nonblocking");
        goto cleanup;
    }

    if (!set_socket_linger(sockfd, 0))
    {
        echo("failed to set socket option");
        goto cleanup;
    }

    conn->sockfd = sockfd;
    if (bind_and_listen(conn)) return true;

cleanup:
    if (close(sockfd) == -1) echo_errno("failed to close socket");
    return false;
}

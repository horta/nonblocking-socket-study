#include "conn.h"
#include "client_config.h"
#include <errno.h>
#include <ev.h>
#include <fcntl.h>
#include <stdio.h>

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

bool conn_init(struct conn *conn)
{
    conn->state = CONN_INIT;

    if ((addr_setup(&conn->addr, IPV4, "100.118.66.95", PORT)))
    {
        puts("failed to setup addr");
        return false;
    }

    int const sockfd = socket(conn->addr.sin_family, SOCK_STREAM, IPPROTO_TCP);
    if (sockfd == -1)
    {
        puts("failed to create socket");
        return false;
    }

    if (!set_socket_reuseaddr(sockfd))
    {
        puts("failed to set socket option");
        goto cleanup;
    }

    if (!set_socket_nonblocking(sockfd))
    {
        puts("failed to set socket nonblocking");
        goto cleanup;
    }

    if (!set_socket_linger(sockfd, 0))
    {
        puts("failed to set socket option");
        goto cleanup;
    }

    conn->sockfd = sockfd;
    return true;

cleanup:
    if (close(sockfd) == -1) perror("failed to close socket");
    return false;
}

bool conn_connect(struct conn *conn)
{
    conn->state = CONN_PEND;
    printf("connecting to server...\n");

    socklen_t size = sizeof(conn->addr);
    if (connect(conn->sockfd, (struct sockaddr *)&conn->addr, size) != -1)
    {
        puts("DEBUG: connected fast");
        return true;
    }

    if (errno == EAGAIN || errno == EINPROGRESS)
    {
        // puts("DEBUG: check connection later");
        return true;
    }

    puts("failed to connect");
    if (close(conn->sockfd) == -1) perror("failed to close socket: ");
    return false;
}

void conn_check_connection(struct conn *conn)
{
    int err = 0;
    socklen_t len = sizeof(err);
    if (getsockopt(conn->sockfd, SOL_SOCKET, SO_ERROR, &err, &len) == -1)
    {
        puts("failed to check socket for connection");
        conn->state = CONN_FAIL;
    }
    if (err)
    {
        puts("failed to connect");
        conn->state = CONN_FAIL;
        return;
    }
#if defined(__FreeBSD__) || defined(BSD)
    // Special case for FreeBSD7, for which send() doesn't do the trick
    struct sockaddr_in junk;
    socklen_t length = sizeof(junk);
    memset(&junk, 0, sizeof(junk));
    conn->state = getpeername(fd, (struct sockaddr *)&junk, &length) == 0
                      ? CONN_RUN
                      : CONN_FAIL;
#else
    // https://stackoverflow.com/a/36254695
    char junk = 0;
    conn->state = send(conn->sockfd, &junk, 0, 0) == 0 ? CONN_RUN : CONN_FAIL;
#endif
}

void conn_close(struct conn *conn)
{
    puts("conn_close");
    if (shutdown(conn->sockfd, SHUT_WR))
        perror("failed to wr-shutdown socket: ");
    ev_sleep(1);
    if (shutdown(conn->sockfd, SHUT_RD))
        perror("failed to rd-shutdown socket: ");
    ev_sleep(1);
    if (close(conn->sockfd) == -1) perror("failed to close socket: ");
}

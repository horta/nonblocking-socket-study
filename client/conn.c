#include "conn.h"
#include "client_config.h"
#include "report.h"
#include <errno.h>
#include <fcntl.h>

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

    if ((addr_setup(&conn->addr, IPV4, "127.0.0.1", PORT)))
    {
        echo("failed to setup addr");
        return false;
    }

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
    return true;

cleanup:
    if (close(sockfd) == -1) echo_errno("failed to close socket");
    return false;
}

bool conn_start_connecting(struct conn *conn)
{
    conn->state = CONN_PEND;
    echo("connecting to server...");

    socklen_t size = sizeof(conn->addr);
    if (connect(conn->sockfd, (struct sockaddr *)&conn->addr, size) != -1)
    {
        echo("DEBUG: connected fast");
        return true;
    }

    if (errno == EAGAIN || errno == EINPROGRESS)
    {
        echo("DEBUG: check connection later");
        return true;
    }

    echo("failed to connect");
    if (close(conn->sockfd) == -1) echo_errno("failed to close socket: ");
    return false;
}

void conn_check_connection(struct conn *conn)
{
    int err = 0;
    socklen_t len = sizeof(err);
    if (getsockopt(conn->sockfd, SOL_SOCKET, SO_ERROR, &err, &len) == -1)
    {
        echo("failed to check socket for connection");
        conn->state = CONN_FAIL;
    }
    if (err)
    {
        echo("failed to connect");
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
    if (close(conn->sockfd) == -1) echo_errno("failed to close socket: ");
}

#include "client.h"
#include "global.h"
#include "report.h"
#include "server_config.h"
#include "socket_include.h"
#include <errno.h>
#include <stdio.h>

struct client
{
    unsigned id;
    struct conn conn;
    struct ev_io read_watcher;
    struct ev_io write_watcher;
    size_t len;
    unsigned char buf[128];
};

static struct client clients[BACKLOG] = {0};
static bool avail[BACKLOG] = {0};

void clients_init(void)
{
    for (unsigned i = 0; i < BACKLOG; ++i)
    {
        avail[i] = true;
        clients[i].len = 128;
    }
}

struct client *clients_next_avail(void)
{
    for (unsigned i = 0; i < BACKLOG; ++i)
    {
        if (avail[i])
        {
            avail[i] = false;
            clients[i].id = i;
            return clients + i;
        }
    }
    return 0;
}

struct conn *client_conn(struct client *client) { return &client->conn; }

static void terminate_client(struct client *cl)
{
    ev_io_stop(global_loop(), &cl->read_watcher);
    ev_io_stop(global_loop(), &cl->write_watcher);
    if (close(cl->conn.sockfd) == -1)
        echo_errno("failed to close connection");
    else
        echo("closed connection");
    avail[cl->id] = true;
}

static void read_callback(EV_P_ ev_io *w, int revents)
{
    echo("read_callback");
    if (EV_ERROR & revents) die("invalid event");

    struct client *cl = (struct client *)w->data;
    ssize_t size = recv(cl->conn.sockfd, cl->buf, cl->len, 0);
    echo("recv size: %lld", size);
    if (size == -1)
    {
        if (errno == ECONNRESET)
        {
            terminate_client(cl);
            return;
        }
        echo_errno("recv error: ");
    }
    if (size <= 0)
    {
        terminate_client(cl);
        return;
    }
    echo("Data(%d): [%.*s]", size, size, (char const *)cl->buf);
    (void)revents;
}

static void write_callback(EV_P_ ev_io *w, int revents)
{
    echo("write_callback");
    if (EV_ERROR & revents) die("invalid event");
}

// As per Unix Network Programming, Volume 1 book says for
// non-blocking socket.
static inline bool ignore_accept_error(void)
{
    return errno == EWOULDBLOCK || errno == ECONNABORTED || errno == EPROTO ||
           errno == EINTR;
}

bool client_accept(struct client *cl, int sockfd)
{
    struct sockaddr_in *addr = &cl->conn.addr;
    socklen_t n = sizeof(*addr);
    cl->conn.sockfd = accept(sockfd, (struct sockaddr *)addr, &n);
    if (cl->conn.sockfd == -1)
    {
        if (!ignore_accept_error())
        {
            echo_errno("failed to accept connection");
            avail[cl->id] = true;
            return false;
        }
    }

    ev_io_init(&cl->read_watcher, read_callback, cl->conn.sockfd, EV_READ);
    ev_io_init(&cl->write_watcher, write_callback, cl->conn.sockfd, EV_WRITE);

    cl->read_watcher.data = cl;
    cl->write_watcher.data = cl;

    ev_io_start(global_loop(), &cl->read_watcher);
    // ev_io_start(global_loop(), &cl->write_watcher);

    echo("accepted connection (non-blocking)");
    return true;
}

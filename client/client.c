#include "addr.h"
#include "client_config.h"
#include "conn.h"
#include "global.h"
#include "report.h"
#include "socket_include.h"
#include "strings.h"
#include "term.h"
#include <errno.h>
#include <ev.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

static struct conn conn = {0};

static ev_signal signal_watcher = {0};
static ev_io write_watcher = {0};
static ev_timer connect_timeout_watcher = {0};

static void stop_watchers(void)
{
    ev_signal_stop(global_loop(), &signal_watcher);
    // ev_timer_stop(global_loop(), &connect_timeout_watcher);
    ev_io_stop(global_loop(), &write_watcher);
}

static void terminate(void)
{
    conn_close(&conn);
    stop_watchers();
}

static void sigint_callback(struct ev_loop *loop, ev_signal *w, int revents)
{
    echo("sigint_callback");
    ev_break(global_loop(), EVBREAK_ALL);
}

static void connect_callback(EV_P_ ev_io *w, int revents)
{
    if (EV_ERROR & revents) die("invalid event");

    conn_check_connection(&conn);
    if (conn.state == CONN_PEND) die("connection still pending");

    if (conn.state == CONN_FAIL)
    {
        echo("connection has failed");
        terminate();
        return;
    }

    if (conn.state == CONN_RUN)
    {
        echo("connected!");
        ev_timer_stop(global_loop(), &connect_timeout_watcher);
        ev_io_start(loop, &write_watcher);
        return;
    }
}

static char const *ptr = 0;
static char const *end = 0;

static void write_callback(EV_P_ ev_io *w, int revents)
{
    echo("write_callback");
    // echo("Send(%u) [%.*s]", (unsigned)(end - ptr), (unsigned)(end - ptr),
    // ptr);

    if (end == ptr) die("zero sized message");
    while (end - ptr > 0)
    {
        ssize_t size = send(conn.sockfd, ptr, end - ptr, 0);
        if (size == -1)
        {
            if (errno == EWOULDBLOCK)
            {
                echo("DEBUG: recv returned EWOULDBLOCK");
                return;
            }
            echo_errno("failed to send data: ");
            echo("stoppping further sending");
            ev_io_stop(global_loop(), &write_watcher);
            return;
        }
        ptr += size;
    }
    // terminate();
    echo("finished sending data");
    echo("stoppping further sending");
    ev_sleep(2);
    ev_io_stop(global_loop(), &write_watcher);
    ev_break(global_loop(), EVBREAK_ALL);
}

static void connect_timeout_callback(EV_P_ ev_timer *w, int revents)
{
    if (EV_ERROR & revents) die("invalid event");
    echo("connecting timed out");
    terminate();
}

#if 0
int main(void)
{
    // ptr = big;
    // end = ptr + strlen(big);

    global_init();
    term_init();
    struct ev_loop *loop = global_loop();

    ev_run(loop, 0);
    term_cleanup();

    return 0;

    if (!conn_init(&conn)) return 1;

    ev_signal_init(&signal_watcher, sigint_callback, SIGINT);
    ev_io_init(&write_watcher, write_callback, conn.sockfd, EV_WRITE);
    // ev_timer_init(&connect_timeout_watcher, connect_timeout_callback,
    //               CONNECT_TIMEOUT, 0);

    ev_signal_start(loop, &signal_watcher);
    ev_io_start(loop, &write_watcher);

    if (!conn_start_connecting(&conn)) return 1;

    // ev_timer_start(loop, &connect_timeout_watcher);

    ev_run(loop, 0);

    terminate();

    return 0;
}
#endif

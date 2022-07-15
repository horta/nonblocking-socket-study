#include "client.h"
#include "conn.h"
#include "global.h"
#include "report.h"
#include "server_config.h"
#include <ev.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

static struct conn conn = {0};
static ev_signal signal_watcher = {0};
static ev_io accept_watcher = {0};

static void accept_callback(EV_P_ ev_io *w, int revents)
{
    if (EV_ERROR & revents) die("invalid event");

    struct client *cl = clients_next_avail();
    if (!cl)
    {
        echo("no space left for incoming client");
        ev_io_stop(global_loop(), &accept_watcher);
        return;
    }

    if (client_accept(cl, conn.sockfd)) echo("new client connected!");
    // ev_io_stop(EV_A_ w);
    // ev_break(EV_A_ EVBREAK_ALL);
}

static void terminate(void)
{
    if (close(conn.sockfd) == -1) echo_errno("failed to close");
}

static void sigint_callback(struct ev_loop *loop, ev_signal *w, int revents)
{
    if (EV_ERROR & revents) die("invalid event");
    // echo("sigint_callback");
    ev_break(global_loop(), EVBREAK_ALL);
}

int main(void)
{
    global_init();
    clients_init();

    conn_listen(&conn);
    echo("Listening to clients...");

    struct ev_loop *loop = global_loop();

    ev_signal_init(&signal_watcher, sigint_callback, SIGINT);
    ev_signal_start(loop, &signal_watcher);

    ev_io_init(&accept_watcher, accept_callback, conn.sockfd, EV_READ);
    ev_io_start(loop, &accept_watcher);

    ev_run(loop, 0);

    terminate();

    echo("Goodbye!");
    return 0;
}

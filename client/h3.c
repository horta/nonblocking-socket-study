#include "h3.h"
#include "conn.h"
#include <errno.h>
#include <ev.h>
#include <stdio.h>

static struct ev_loop *loop = 0;
static ev_tstamp startup_timestamp = {0};
static ev_timer timeout_watcher = {0};
static ev_io send_watcher = {0};
static ev_idle idle_watcher = {0};
static struct conn conn = {0};

static struct
{
    char const *data;
    char const *pos;
    char const *end;
} msg;

static void sigint_callback(struct ev_loop *loop, ev_signal *w, int revents)
{
    ev_break(loop, EVBREAK_ALL);
}

static void idle_callback(struct ev_loop *loop, ev_idle *w, int revents)
{
    ev_sleep(0.1);
}

static void send_callback(EV_P_ ev_io *w, int revents)
{
    puts("send_callback!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!");
    if (msg.end == msg.pos) puts("zero sized message");
    while (msg.end - msg.pos > 0)
    {
        ssize_t size = send(conn.sockfd, msg.pos, msg.end - msg.pos, 0);
        if (size == -1)
        {
            if (errno == EWOULDBLOCK)
            {
                puts("DEBUG: recv returned EWOULDBLOCK");
                return;
            }
            perror("failed to send data: ");
            ev_io_stop(loop, &send_watcher);
            return;
        }
        msg.pos += size;
    }
    puts("finished sending data");
    ev_timer_stop(loop, &timeout_watcher);
    ev_io_stop(loop, &send_watcher);
}

static bool check_connection(void)
{
    int error = 0;
    socklen_t len = sizeof(error);
    int retval = getsockopt(conn.sockfd, SOL_SOCKET, SO_ERROR, &error, &len);
    if (retval != 0)
    {
        /* there was a problem getting the error code */
        fprintf(stderr, "error getting socket error code: %s\n",
                strerror(retval));
        return false;
    }

    if (error != 0)
    {
        /* socket has a non zero error status */
        fprintf(stderr, "socket error: %s\n", strerror(error));
        return false;
    }

    return true;
}

static void timeout_callback(EV_P_ ev_timer *w, int revents)
{
    puts("timeout, giving up on sending the message");
    // check_connection();
    ev_io_stop(loop, &send_watcher);
    ev_timer_stop(loop, &timeout_watcher);
}

bool h3_init(void)
{
    if (!conn_init(&conn)) return false;
    loop = EV_DEFAULT;
    startup_timestamp = ev_now(loop);
    ev_idle_init(&idle_watcher, idle_callback);
    ev_io_init(&send_watcher, send_callback, conn.sockfd, EV_WRITE);
    ev_timer_init(&timeout_watcher, timeout_callback, 2.0, 0.);
    ev_set_priority(&idle_watcher, EV_MINPRI);
    return true;
}

void h3_start(void)
{
    ev_idle_start(loop, &idle_watcher);
    ev_run(loop, 0);
}

bool h3_connect(void) { return conn_connect(&conn); }

void h3_send(char const *data)
{
    msg.data = data;
    msg.pos = data;
    msg.end = msg.pos + strlen(data);
    ev_io_start(loop, &send_watcher);

    if (!check_connection()) ev_io_stop(loop, &send_watcher);

    ev_timer_start(loop, &timeout_watcher);
}

double h3_now(void) { return ev_now(loop) - startup_timestamp; }

void h3_stop(void)
{
    ev_idle_stop(loop, &idle_watcher);
    ev_io_stop(loop, &send_watcher);
    conn_close(&conn);
    ev_sleep(2);
    ev_break(loop, EVBREAK_ALL);
}
// void h3_sigint(void) { ev_feed_event(loop, &signal_watcher, 0); }

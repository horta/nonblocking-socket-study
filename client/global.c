#include "global.h"
#include <ev.h>

static struct ev_loop *loop = 0;
static ev_signal signal_watcher = {0};
static ev_tstamp startup_timestamp = {0};

static void sigint_callback(struct ev_loop *loop, ev_signal *w, int revents)
{
    ev_signal_stop(loop, &signal_watcher);
    ev_break(loop, EVBREAK_ALL);
}

void global_init(void)
{
    loop = EV_DEFAULT;
    startup_timestamp = ev_now(loop);
    ev_signal_init(&signal_watcher, sigint_callback, SIGINT);
    ev_signal_start(loop, &signal_watcher);
}

struct ev_loop *global_loop(void) { return loop; }

double global_now(void) { return ev_now(loop) - startup_timestamp; }

void global_sigint(void) { ev_feed_event(loop, &signal_watcher, 0); }

#include "global.h"
#include <ev.h>

static struct ev_loop *loop = 0;
static ev_tstamp startup_timestamp = {0};

void global_init(void)
{
    loop = EV_DEFAULT;
    startup_timestamp = ev_now(loop);
}

struct ev_loop *global_loop(void) { return loop; }

double global_now(void) { return ev_now(loop) - startup_timestamp; }

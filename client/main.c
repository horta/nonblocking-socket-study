#include "conn.h"
#include "h3.h"
#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

pthread_t h3_thread_id = {0};

void *h3_thread_start(void *ptr)
{
    h3_start();
    pthread_exit(0);
    return 0;
}

#define MAXLINE 1024

static char line[MAXLINE] = {0};

static struct conn conn = {0};

void connect_cmd(void)
{
    if (!h3_connect()) printf("failed to start connecting\n");
}

void send_cmd(char const *data) { h3_send(data); }

static char *shrink_line(char const *start, char *end)
{
    while (start < end && *(end - 1) == '\n')
        --end;
    *end = 0;
    return end;
}

int main(void)
{
    if (!h3_init()) return 1;
    pthread_create(&h3_thread_id, 0, *h3_thread_start, 0);

    while (fgets(line, sizeof line, stdin))
    {
        shrink_line(line, line + strlen(line));

        if (!strncmp(line, "q", 1)) break;
        if (!strncmp(line, "c ", 1)) connect_cmd();
        if (!strncmp(line, "s ", 1)) send_cmd(line + 2);
    }

    h3_stop();
    pthread_join(h3_thread_id, 0);
    pthread_exit(0);

    // term_init();
    // struct ev_loop *loop = global_loop();
    //
    // ev_run(loop, 0);
    // term_cleanup();
    // return 0;
}

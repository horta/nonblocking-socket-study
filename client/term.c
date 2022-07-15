#include "global.h"
#include "report.h"
#include <errno.h>
#include <ev.h>
#include <fcntl.h>
#include <stdbool.h>
#include <stdio.h>
#include <unistd.h>

#define MAXLINE 1024

struct line
{
    unsigned size;
    char buff[MAXLINE];
    bool over;
    char *pos;
};

static struct term
{
    int fd;
    ev_io watcher;
    struct line line;
} term;

static void init_line(struct line *line)
{
    term.line.size = 0;
    memset(term.line.buff, 0, MAXLINE);
    term.line.over = false;
    term.line.pos = term.line.buff;
}

static void reset_line(struct line *line)
{
    line->size = 0;
    line->buff[0] = 0;
    line->over = false;
    line->pos = line->buff;
}

static void replace_newline(unsigned size, char *msg)
{
    for (unsigned i = 0; i < size; ++i)
    {
        if (msg[i] == '\n') msg[i] = '$';
    }
}
static void process_input(unsigned size, char *msg)
{
    replace_newline(size, msg);
    echo("input[%s]", msg);
    reset_line(&term.line);
}

static void read_input(EV_P_ ev_io *w, int revents)
{
    if (EV_ERROR & revents) die("invalid event");

    char const *end = term.line.buff + MAXLINE - 1;

    bool first_read = true;
    while (!term.line.over)
    {
        if (end == term.line.pos)
        {
            echo("line is too long");
            reset_line(&term.line);
            break;
        }

        ssize_t n = read(term.fd, term.line.pos, end - term.line.pos);
        if (n < 0)
        {
            if (errno == EWOULDBLOCK)
            {
                if (first_read)
                {
                    echo("would block but shouldn't");
                    break;
                }
                term.line.over = true;
            }
            else
            {
                echo_errno("failed to read from stdin: ");
                break;
            }
        }
        else if (n == 0)
            term.line.over = true;
        else
            term.line.pos += n;
        first_read = false;
    }

    if (term.line.over)
    {
        *term.line.pos = 0;
        process_input(term.line.pos - term.line.buff, term.line.buff);
    }
}

static void set_nonblocking(int fd)
{
    int val = fcntl(fd, F_GETFL, 0);
    fcntl(fd, F_SETFL, val | O_NONBLOCK);
}

void term_init(void)
{
    term.fd = STDIN_FILENO;
    init_line(&term.line);
    set_nonblocking(term.fd);
    ev_io_init(&term.watcher, read_input, term.fd, EV_READ);
    ev_io_start(global_loop(), &term.watcher);
}

void term_cleanup(void) { ev_io_stop(global_loop(), &term.watcher); }

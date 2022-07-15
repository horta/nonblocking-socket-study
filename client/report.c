#include "report.h"
#include "global.h"
#include <errno.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define TSTEMP_FMT "[% 14.6f] "

void die(char const *fmt, ...)
{
    va_list params;
    va_start(params, fmt);

    fprintf(stderr, TSTEMP_FMT, global_now());
    fprintf(stderr, "%s:", "FATAL");
    vfprintf(stderr, fmt, params);
    fputc('\n', stderr);

    va_end(params);
    exit(1);
}

void echo(char const *fmt, ...)
{
    va_list params;
    va_start(params, fmt);

    fprintf(stdout, TSTEMP_FMT, global_now());
    vfprintf(stdout, fmt, params);
    fputc('\n', stdout);

    va_end(params);
}

void echo_errno(char const *fmt, ...)
{
    char const *err_msg = strerror(errno);

    va_list params;
    va_start(params, fmt);

    fprintf(stderr, TSTEMP_FMT, global_now());
    vfprintf(stderr, fmt, params);
    fputs(err_msg, stderr);
    fputc('\n', stderr);

    va_end(params);
}

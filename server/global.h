#ifndef GLOBAL_H
#define GLOBAL_H

struct ev_loop;

void global_init(void);
struct ev_loop *global_loop(void);
double global_now(void);

#endif

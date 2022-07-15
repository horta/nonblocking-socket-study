#ifndef H3_H
#define H3_H

#include <stdbool.h>

bool h3_init(void);
void h3_start(void);
bool h3_connect(void);
void h3_send(char const *data);
double h3_now(void);
void h3_stop(void);

#endif

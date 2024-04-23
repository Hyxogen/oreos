#ifndef KERNEL_H
#define KERNEL_H

#include <kernel/types.h>

#define ANSI_ESCAPE 27

void term_put(int c);
void term_write(const char *data, size_t n);
void term_print(const char *str);

#endif

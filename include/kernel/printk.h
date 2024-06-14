#ifndef KERNEL_KERNEL_H
#define KERNEL_KERNEL_H

#include <kernel/tty.h>
#include <stdarg.h>

void printk_set_sink(struct term *term);
__attribute__((format(printf, 1, 2))) int printk(const char *fmt, ...);
int vprintk(const char *fmt, va_list args);

#endif

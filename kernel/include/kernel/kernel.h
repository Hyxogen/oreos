#ifndef _KERNEL_KERNEL
#define _KERNEL_KERNEL

#include <kernel/tty.h>

#define ARRAY_SIZE(arr) (sizeof(arr)/sizeof((arr)[0]))

__attribute__ ((noreturn)) void panic(const char *fmt, ...);

void printk_set_sink(struct term *term);
__attribute__ ((format(printf,1, 2))) int printk(const char *fmt, ...);
#endif

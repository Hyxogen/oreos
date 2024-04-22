#ifndef _KERNEL_KERNEL
#define _KERNEL_KERNEL

__attribute__ ((noreturn)) void panic(const char *fmt, ...);

__attribute__ ((format(printf,1, 2))) int printk(const char *fmt, ...);
#endif

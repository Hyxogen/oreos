#ifndef _KERNEL_KERNEL
#define _KERNEL_KERNEL

#define ARRAY_SIZE(arr) (sizeof(arr) / sizeof((arr)[0]))

__attribute__((noreturn)) void panic(const char *fmt, ...);

__attribute__((noreturn)) void reset(void);
__attribute__((noreturn)) void halt(void);
#endif

#ifndef __KERNEL_KERNEL
#define __KERNEL_KERNEL

#define ARRAY_SIZE(arr) (sizeof(arr) / sizeof((arr)[0]))

//TODO common function that will trap the debugger when one of these is called?
__attribute__((noreturn)) void panic(const char *fmt, ...);
void oops(const char *fmt, ...);

__attribute__((noreturn)) void reset(void);
__attribute__((noreturn)) void halt(void);

void dump_registers(void);
#endif

#ifndef __KERNEL_KERNEL
#define __KERNEL_KERNEL

#define ARRAY_SIZE(arr) (sizeof(arr) / sizeof((arr)[0]))

//TODO common function that will trap the debugger when one of these is called?
__attribute__((noreturn)) void panic(const char *fmt, ...);
void oops(const char *fmt, ...);

__attribute__ ((noreturn)) void unreachable(void);

struct mb2_info *mb2_get_info(void);
#endif

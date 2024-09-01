#ifndef __KERNEL_ARCH_I386_IO_H
#define __KERNEL_ARCH_I386_IO_H

#include <kernel/types.h>

void outb(u16 port, u8 b);
u8 inb(u16 port);
void io_wait(void);

#endif

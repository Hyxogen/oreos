#ifndef __KERNEL_SYSCALL_H
#define __KERNEL_SYSCALL_H

#include <stddef.h>
#include <kernel/platform.h>

#define SYSCALL_READ 0x03
#define SYSCALL_WRITE 0x04
#define SYSCALL_SCHED_YIELD 0x9E

int do_syscall(struct cpu_state *state);

i32 write(int fd, const void *buf, size_t nbytes);
i32 read(int fd, void *buf, size_t nbytes);

#endif

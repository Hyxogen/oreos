#ifndef __KERNEL_SYSCALL_H
#define __KERNEL_SYSCALL_H

#include <stddef.h>
#include <kernel/platform.h>

int do_syscall(struct cpu_state *state);
i32 write(int fd, const void *buf, size_t nbytes);

#endif

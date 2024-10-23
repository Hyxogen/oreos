#ifndef __KERNEL_SYSCALL_H
#define __KERNEL_SYSCALL_H

#include <stddef.h>
#include <kernel/platform.h>

#define SYSCALL_EXIT 0x01
#define SYSCALL_FORK 0x02
#define SYSCALL_READ 0x03
#define SYSCALL_WRITE 0x04
#define SYSCALL_SCHED_YIELD 0x9E

int do_syscall(struct cpu_state *state);

i32 syscall_exit(int error_code);
i32 syscall_fork(void);
i32 syscall_write(int fd, const void *buf, size_t nbytes);
i32 syscall_read(int fd, void *buf, size_t nbytes);
i32 syscall_mmap(uintptr_t addr, size_t len, int prot, int flags, int fd, i32 off);

#endif

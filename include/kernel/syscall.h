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

i32 syscall_exit(struct cpu_state *state, int error_code);
i32 syscall_fork(struct cpu_state *state);
i32 syscall_write(struct cpu_state *state, int fd, const void *buf, size_t nbytes);
i32 syscall_read(struct cpu_state *state, int fd, void *buf, size_t nbytes);
i32 syscall_mmap(struct cpu_state *state, uintptr_t addr, size_t len, int prot, int flags, int fd, i32 off);
i32 syscall_signal(struct cpu_state *state, int signum, void (*handler)(int));
i32 syscall_sigreturn(struct cpu_state *state);

#endif

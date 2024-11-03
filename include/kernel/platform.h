#ifndef __KERNEL_PLATFORM_H
#define __KERNEL_PLATFORM_H

#include <stdbool.h>
#include <kernel/types.h>

#define SYSCALL_IRQ 0x80

struct cpu_state;

__attribute__((noreturn))
void idle(void);
__attribute__((noreturn))
void reset(void);

void halt(void);

void short_wait(void);

__attribute__ ((noreturn))
void return_from_irq(struct cpu_state *state);

bool is_from_userspace(const struct cpu_state *state);
bool is_from_uaccess(const struct cpu_state *state);

__attribute__((warn_unused_result))
int put_user1(void *dest, u8 val);
__attribute__((warn_unused_result))
int get_user1(u8 *dest, const void *src);
__attribute__((warn_unused_result))
int copy_to_user(void *dest, const void *src, size_t nbytes);
__attribute__((warn_unused_result))
int copy_from_user(void *dest, const void *src, size_t nbytes);

#endif

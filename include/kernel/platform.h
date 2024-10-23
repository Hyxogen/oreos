#ifndef __KERNEL_PLATFORM_H
#define __KERNEL_PLATFORM_H

#include <stdbool.h>
#include <kernel/types.h>

/* DEPRECATED: stack_top will be removed when userspace programs are properly
 * implemented */
__attribute__((deprecated))
extern u8 _stack_top;

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

void *put_user1(void *dest, u8 val);

#endif

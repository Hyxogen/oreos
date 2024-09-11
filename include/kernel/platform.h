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

void disable_irqs(void);
void enable_irqs(void);

unsigned irq_get_id(const struct cpu_state *state);
bool irq_should_ignore(unsigned irq);
bool irq_returning_to_userspace(const struct cpu_state *state);

__attribute__((noreturn))
void idle(void);
__attribute__((noreturn))
void reset(void);

void halt(void);

#endif

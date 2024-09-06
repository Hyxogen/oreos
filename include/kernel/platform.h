#ifndef __KERNEL_PLATFORM_H
#define __KERNEL_PLATFORM_H

struct cpu_state;

void disable_irqs(void);
void enable_irqs(void);

__attribute__((noreturn))
void idle(void);
__attribute__((noreturn))
void reset(void);

void halt(void);

#endif

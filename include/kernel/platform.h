#ifndef __KERNEL_PLATFORM_H
#define __KERNEL_PLATFORM_H

#include <stdbool.h>

struct cpu_state;

void disable_irqs(void);
void enable_irqs(void);

unsigned irq_get_id(const struct cpu_state *state);
bool irq_should_ignore(unsigned irq);

__attribute__((noreturn))
void idle(void);
__attribute__((noreturn))
void reset(void);

void halt(void);

#endif

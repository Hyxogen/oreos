#ifndef __KERNEL_IRQ_H
#define __KERNEL_IRQ_H

#include <kernel/types.h>
#include <kernel/platform.h>

enum irq_result {
	IRQ_CONTINUE,
	IRQ_PANIC,
};

int irq_register_handler(
    u8 irq, enum irq_result (*handler)(u8 irq, struct cpu_state *state, void *),
    void *ctx);

i16 irq_get_free_irq(void);

unsigned irq_get_id(const struct cpu_state *state);
bool irq_should_ignore(unsigned irq);
bool irq_returning_to_userspace(const struct cpu_state *state);


void disable_irqs(void);
void enable_irqs(void);

#endif

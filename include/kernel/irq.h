#ifndef __KERNEL_IRQ_H
#define __KERNEL_IRQ_H

#include <kernel/types.h>
#include <kernel/platform.h>
#include <kernel/acpi/acpi.h>

enum irq_result {
	/* TODO give IRQ_IGNORE a better name, it's supposed to indicate that
	 * the IRQ was not handled and it should continue to execute other
	 * handlers */
	IRQ_IGNORE,
	IRQ_CONTINUE,
	IRQ_PANIC,
};

int irq_register_handler(
    u8 irq, enum irq_result (*handler)(u8 irq, struct cpu_state *state, void *),
    void *ctx);

i16 irq_get_free_irq(void);

unsigned irq_get_id(const struct cpu_state *state);
bool irq_returning_to_userspace(const struct cpu_state *state);
bool irq_is_reserved(u8 irqn);

void disable_irqs(void);
void enable_irqs(void);

void init_interrupts(struct acpi_table *table);
#endif

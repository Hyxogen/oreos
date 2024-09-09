#ifndef __KERNEL_TIMER_H
#define __KERNEL_TIMER_H

#include <kernel/types.h>
#include <kernel/acpi/acpi.h>

#define TIMER_IRQ 0x48

void timer_sleep(u32 millis);
u32 timer_poll(void);
u32 timer_schedule_irq(u32 millis);
void timer_init(struct acpi_table *table);
bool timer_tick(void);

void timer_eoi(void);

void timer_sched_int(u32 millis);

#endif

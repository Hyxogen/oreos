#ifndef __KERNEL_TIMER_H
#define __KERNEL_TIMER_H

#include <kernel/types.h>
#include <kernel/acpi.h>

void timer_sleep(u32 millis);
u32 timer_poll(void);
u32 timer_schedule_irq(u32 millis);
void timer_init(struct acpi_table *table);
void timer_tick(void);

#endif

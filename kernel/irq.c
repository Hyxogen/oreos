#include <kernel/kernel.h>
#include <kernel/platform.h>
#include <kernel/timer.h>
#include <kernel/sched.h>
#include <kernel/printk.h>
#include <kernel/debug.h>
#include <kernel/libc/assert.h>

void* irq_callback(struct cpu_state *state)
{
	int irq = irq_get_id(state);
	if (irq_should_ignore(irq))
		return state;

	switch (irq) {
	case TIMER_IRQ:
		if (timer_tick()) {
			state = do_schedule(state);
			assert(state);
		}
		timer_eoi(); /* TODO this sends LAPIC EOI, should this be at other places aswell? */
		break;
	default:
		printk("irq stackstrace:\n");
		dump_stacktrace_at(state);
		//printk("cpu state:\n");
		//dump_state(state);
		panic("unhandled interrupt: 0x%x (%d)\n", irq, irq);
	}
	return state;
}


#include <kernel/kernel.h>
#include <kernel/platform.h>
#include <kernel/timer.h>
#include <kernel/sched.h>
#include <kernel/printk.h>
#include <kernel/debug.h>
#include <kernel/libc/assert.h>

//TODO remove
#include <kernel/arch/i386/platform.h>

void* irq_callback(struct cpu_state *state)
{
	_reload_segments(I386_KERNEL_CODE_SELECTOR, I386_KERNEL_DATA_SELECTOR);
	int irq = irq_get_id(state);
	if (irq_should_ignore(irq))
		return state;

	switch (irq) {
	case TIMER_IRQ:
		timer_tick();
		timer_eoi(); /* TODO this sends LAPIC EOI, should this be at other places aswell? */
		break;
	case 0xab: /* TODO stupid temp code, remove */
		state = do_schedule(state);
		assert(state);
		return state;
	default:
		printk("irq stackstrace:\n");
		dump_stacktrace_at(state);
		printk("cpu state:\n");
		dump_state(state);
		panic("unhandled interrupt: 0x%x (%d)\n", irq, irq);
	}
	if (irq_returning_to_userspace(state)) {
		if (timer_poll() == 0) {
			state = do_schedule(state);
			assert(state);
		}
	}
	return state;
}


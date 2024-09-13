#include <kernel/irq.h>
#include <kernel/kernel.h>
#include <kernel/platform.h>
#include <kernel/timer.h>
#include <kernel/sched.h>
#include <kernel/printk.h>
#include <kernel/debug.h>
#include <kernel/libc/assert.h>
#include <kernel/syscall.h>
#include <kernel/malloc/malloc.h>

struct irq_handler {
	enum irq_result (*handler)(u8 irq, struct cpu_state *state, void*);
	void *ctx;

	struct irq_handler *_next;
};

static struct irq_handler *_handlers[256];

int irq_register_handler(
    u8 irq, enum irq_result (*handler)(u8 irq, struct cpu_state *state, void *),
    void *ctx)
{
	struct irq_handler *link = kmalloc(sizeof(*handler));
	if (!link)
		return -1;

	link->handler = handler;
	link->ctx = ctx;
	link->_next = NULL;

	struct irq_handler *cur = _handlers[irq];
	if (cur) {
		while (cur->_next)
			cur = cur->_next;
		cur->_next = link;
	} else {
		_handlers[irq] = link;
	}

	return 0;
}

static bool irq_exec_handlers(i8 irq, struct cpu_state *state)
{
	bool handled = false;
	struct irq_handler *cur = _handlers[irq];

	while (cur) {
		handled = true;
		enum irq_result res = cur->handler(irq, state, cur->ctx);

		switch (res) {
		case IRQ_CONTINUE:
			break;
		case IRQ_PANIC:
			panic("irq handlers signalled panic\n");
		}

		cur = cur->_next;
	}
	return handled;
}

i16 irq_get_free_irq(void)
{
	// TODO don't hardcode 50, just register the first few
	for (u16 i = 50; i < ARRAY_SIZE(_handlers); i++) {
		if (irq_is_reserved(i))
			continue;
		if (_handlers[i])
			continue;
		return i;
	}
	return -1;
}

void* irq_callback(struct cpu_state *state)
{
	int irq = irq_get_id(state);

	switch (irq) {
	case TIMER_IRQ:
		timer_tick();
		timer_eoi(); /* TODO this sends LAPIC EOI, should this be at other places aswell? */
		break;
	case SYSCALL_IRQ:
		do_syscall(state);
		break;
	case 0xab: /* TODO stupid temp code, remove */
		state = sched_schedule(state);
		assert(state);
		return state;
	default:
		if (irq_exec_handlers(irq, state))
			break;
		printk("irq stackstrace:\n");
		dump_stacktrace_at(state);
		printk("cpu state:\n");
		dump_state(state);
		panic("unhandled interrupt: 0x%x (%d)\n", irq, irq);
	}

	if (irq_returning_to_userspace(state)) {
		if (sched_should_preempt()) {
			state = sched_schedule(state);
			assert(state);
		}
	}
	return state;
}

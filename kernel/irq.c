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

__attribute__((noreturn))
static void irq_panic(u8 irq, struct cpu_state *state, const char *msg)
{
	assert(0);
	printk("irq: %u\n", irq);
	printk("irq stackstrace:\n");
	dump_stacktrace_at(state);
	printk("cpu state:\n");
	dump_state(state);
	panic("%s", msg);
}

static bool irq_exec_handlers(u8 irq, struct cpu_state *state)
{
	bool handled = false;
	struct irq_handler *cur = _handlers[irq];

	while (cur) {
		enum irq_result res = cur->handler(irq, state, cur->ctx);

		switch (res) {
		case IRQ_CONTINUE:
			handled = true;
			/* fallthrough */
		case IRQ_IGNORE:
			break;
		case IRQ_PANIC:
			irq_panic(irq, state, "irq handler signalled panic\n");
		}

		cur = cur->_next;
	}
	return handled;
}

i16 irq_get_free_irq(void)
{
	for (u16 i = 0; i < ARRAY_SIZE(_handlers); i++) {
		if (irq_is_reserved(i))
			continue;
		if (_handlers[i])
			continue;
		return i;
	}
	return -1;
}

/* TODO mark this function as noreturn */
void* irq_callback(struct cpu_state *state)
{
	int irq = irq_get_id(state);

	if (!irq_exec_handlers(irq, state)) {
		irq_panic(irq, state, "unhandled irq");
	}

	sched_resume(state);
}

static enum irq_result irq_on_syscall(u8 irq, struct cpu_state *state, void *dummy)
{
	(void)irq;
	(void)dummy;
	do_syscall(state);
	return IRQ_CONTINUE;
}

void init_irq_handler(struct acpi_table *table);

void init_interrupts(struct acpi_table *table)
{
	init_irq_handler(table);
	irq_register_handler(SYSCALL_IRQ, irq_on_syscall, NULL);
}

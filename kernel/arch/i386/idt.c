#include <kernel/kernel.h>
#include <kernel/types.h>
#include <kernel/printk.h>
#include <kernel/acpi/acpi.h>
#include <kernel/arch/i386/platform.h>
#include <kernel/sched.h>
#include <kernel/libc/assert.h>
#include <kernel/irq.h>

#include <kernel/arch/i386/apic.h>
#include <kernel/timer.h>

#define IDT_SIZE_16BIT 0x06
#define IDT_SIZE_32BIT 0x0e

#define IDT_TYPE_INTR 0x01
#define IDT_TYPE_TRAP 0x00

#define IDT_FLAG_PRESENT(x) (0x80 * (x))
#define IDT_FLAG_RING(x) (((x) & 0x03) << 0x05)
#define IDT_FLAG_TYPE(size, type) ((size) | (type))

#define IDT_INTR_FLAGS(ring) (IDT_FLAG_PRESENT(1) | IDT_FLAG_RING(ring) | IDT_FLAG_TYPE(IDT_SIZE_32BIT, IDT_TYPE_INTR))

extern char vector_0_handler[];

struct idtr {
	u16 limit;
	u32 base;
} __attribute__((packed));

static u64 __idt[256];

static u64 encode_idt(u32 offset, u16 selector, u8 flags)
{
	u64 res = 0;

	res |= offset & 0xffff0000;
	res |= flags << 8;

	res <<= 32;

	res |= selector << 16;
	res |= offset & 0x0000ffff;

	return res;
}

static void load_idt(void)
{
	struct idtr idtr;

	idtr.limit = sizeof(__idt) - 1;
	idtr.base = (u32) (uintptr_t) __idt;
	__asm__ volatile("lidt %0" : : "m"(idtr));
}

unsigned irq_get_id(const struct cpu_state *state)
{
	return state->vec_num;
}

static enum irq_result irq_spurrious_handler(u8 irq, struct cpu_state *state, void *dummy)
{
	(void) irq;
	(void) state;
	(void) dummy;
	return IRQ_CONTINUE;
}

bool irq_is_reserved(u8 irqn)
{
	return irqn <= 0x1F;
}

static void register_handlers(void)
{
	/*
	 * [0x20-0x30) PIC spurrious interrupts
	 *
	 */
	for (u8 irq = 0x20; irq < 0x30; irq++) {
		irq_register_handler(irq, irq_spurrious_handler, NULL);
	}
	irq_register_handler(LAPIC_SPURRIOUS_IRQN, irq_spurrious_handler, NULL);
}

static void init_idt(void)
{
	for (unsigned i = 0; i < ARRAY_SIZE(__idt); i++) {
		unsigned ring = 0;
		if (i == SYSCALL_IRQ)
			ring = 3;

		__idt[i] = encode_idt((u32) vector_0_handler + i * 16, I386_KERNEL_CODE_SELECTOR, IDT_INTR_FLAGS(ring));
	}
	load_idt();
}

void init_irq_handler(struct acpi_table *table)
{
	init_idt();

	struct madt *madt = (struct madt*) acpi_find(table, ACPI_TAG_APIC);
	if (!madt)
		panic("no madt\n");

	init_apic(madt);

	register_handlers();
	__enable_irqs();
}

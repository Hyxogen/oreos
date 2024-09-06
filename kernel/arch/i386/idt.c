#include <kernel/kernel.h>
#include <kernel/types.h>
#include <kernel/printk.h>
#include <kernel/acpi/acpi.h>
#include <kernel/arch/i386/platform.h>
#include <kernel/sched.h>
#include <kernel/libc/assert.h>

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

static void dump_state(const struct cpu_state *state)
{

	printk("edi: 0x%08lx ", state->edi);
	printk("esi: 0x%08lx ", state->esi);
	printk("ebp: 0x%08lx\n", state->ebp);
	printk("esp: 0x%08lx ", state->esp);
	printk("ebx: 0x%08lx ", state->ebx);
	printk("edx: 0x%08lx\n", state->edx);
	printk("ecx: 0x%08lx ", state->ecx);
	printk("eax: 0x%08lx ", state->eax);
	printk("eip: 0x%08lx\n", state->eip);
	printk("cs: 0x%04hx\n", state->cs);

	printk("vec_num: 0x%08lx ", state->vec_num);
	printk("err_code: 0x%08lx\n", state->err_code);
}

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
	idtr.base = (u32) __idt;
	__asm__ volatile("lidt %0" : : "m"(idtr));
}

void dump_stacktrace_from(const void *ebp);

void* irq_callback(struct cpu_state *state)
{
	switch (state->vec_num) {
	case 0x48:
		//printk("timer tick\n");
		if (timer_tick()) {
			state = do_schedule(state);
			assert(state);
		}
		lapic_eoi(); //TODO where to place this?
		break;
	case 0x20 ... 0x28: /* PIC spurrious IRQ */
	case 0xff: /* APIC spurrious IRQ */
		break;
	default:
		printk("irq stackstrace:\n");
		dump_stacktrace_from((void*) state->ebp);
		printk("cpu state:\n");
		dump_state(state);
		panic("unhandled interrupt: 0x%x (%d)\n", state->vec_num, state->vec_num);
	}
	return state;
}

void init_irq_handler(struct acpi_table *table)
{
	for (unsigned i = 0; i < ARRAY_SIZE(__idt); i++) {
		__idt[i] = encode_idt((u32) vector_0_handler + i * 16, 0x08, IDT_INTR_FLAGS(0));
	}
	load_idt();

	struct madt *madt = (struct madt*) acpi_find(table, ACPI_TAG_APIC);
	if (!madt)
		panic("no madt\n");

	apic_init(madt);
	enable_irqs();
}

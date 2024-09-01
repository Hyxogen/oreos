#include <kernel/kernel.h>
#include <kernel/types.h>
#include <kernel/printk.h>
#include <kernel/acpi.h>

#include <kernel/arch/i386/apic.h>

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

struct cpu_state {
	u32 edi;
	u32 esi;
	u32 ebp;
	u32 esp;
	u32 ebx;
	u32 edx;
	u32 ecx;
	u32 eax;

	u32 vec_num;
	u32 err_code;

	u32 eip;
	u16 cs;
};

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

void irq_callback(struct cpu_state *state)
{
	(void) state;
	printk("got a irq: vec_num: 0x%lx, err: 0x%lx\n", state->vec_num,
	       state->err_code);
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

	apic_init(madt); //TODO pass MADT
}

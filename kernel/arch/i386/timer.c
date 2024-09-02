#include <stdatomic.h>
#include <kernel/timer.h>
#include <kernel/arch/i386/io.h>
#include <kernel/arch/i386/apic.h>
#include <lib/assert.h>
#include <kernel/mmu.h>
#include <kernel/printk.h>

#define PIT_CH0_DATA_PORT 0x40
#define PIT_CH1_DATA_PORT 0x41
#define PIT_CH2_DATA_PORT 0x42
#define PIT_MODECMD_REG 0x43

#define PIT_CMD_CH0 0x00
#define PIT_CMD_CH1 0x60
#define PIT_CMD_CH2 0x80
#define PIT_CMD_READBACK 0xe0

#define PIT_ACCESS_LATCH 0x00
#define PIT_ACCESS_LOBYTE 0x10
#define PIT_ACCESS_HIBYTE 0x20
#define PIT_ACCESS_LOHIBYTE 0x30

#define PIT_OP_IRQ 0x00
#define PIT_OP_HW_RETRIGGER_ONESHOT 0x02
#define PIT_OP_RATEGEN 0x04
#define PIT_OP_SQUAREWAVE_GEN 0x06
#define PIT_OP_SW_STROBE 0x08
#define PIT_OP_HW_STROBE 0x0a

#define PIT_MODE_BINARY 0x00
#define PIT_MODE_BCD 0x01

static void pit_setup(u16 count)
{
	outb(PIT_MODECMD_REG, PIT_CMD_CH0 | PIT_ACCESS_LOHIBYTE |
				  PIT_OP_RATEGEN | PIT_MODE_BINARY);
	outb(PIT_CH0_DATA_PORT, count & 0xff);
	outb(PIT_CH0_DATA_PORT, count >> 8);
}

static atomic_uint_least32_t  __ticks = 0;

void timer_tick(void)
{
	if (timer_poll() > 0)
		atomic_fetch_sub_explicit(&__ticks, 1, memory_order_relaxed);
}

u32 timer_poll(void)
{
	return atomic_load_explicit(&__ticks, memory_order_relaxed);
}

u32 timer_set(u32 ticks)
{
	return atomic_exchange_explicit(&__ticks, ticks, memory_order_relaxed);
}

void timer_sleep(u32 millis)
{
	timer_set(millis);
	while (timer_poll())
		__asm__ volatile("hlt");
}

void timer_init(struct acpi_table *table)
{
	(void)table;
	pit_setup((u16)-1);

	struct madt *madt = (struct madt*) acpi_find(table, ACPI_TAG_APIC);
	assert(madt);
	struct madt_ioapic *ioapic = (struct madt_ioapic*) madt_find(madt, MADT_TYPE_IOAPIC);
	assert(ioapic);
	struct madt_lapic *lapic = (struct madt_lapic*) madt_find(madt, MADT_TYPE_LAPIC);
	assert(lapic);

	madt_dump(madt);

	//TODO make sure to get currect ioapic
	
	//TODO unmap ioapic_addr
	void *p = mmu_map(NULL, ioapic->ioapic_addr, MMU_PAGESIZE, MMU_ADDRSPACE_KERNEL, 0);
	assert(p != MMU_MAP_FAILED);

	u64 v = IOAPIC_PRIO_NORMAL | IOAPIC_DEST_PHYSICAL |
		IOAPIC_POLARITY_HIGHACTIVE | IOAPIC_TRIGGER_EDGE |
		IOAPIC_PHYSICAL_ID(lapic->lapic_id) | IOAPIC_VECTOR(0x48);

	printk("ioapic: 0x%016llx\n", v);
	ioapic_set_redir(p, 2, v);
}

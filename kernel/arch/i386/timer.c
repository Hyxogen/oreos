#include <stdatomic.h>
#include <kernel/timer.h>
#include <kernel/arch/i386/io.h>
#include <kernel/arch/i386/apic.h>
#include <kernel/libc/assert.h>
#include <kernel/mmu.h>
#include <kernel/printk.h>
#include <kernel/platform.h>

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

#define PIT_CH0_IRQ 0x0

static void pit_setup(u16 count)
{
	outb(PIT_MODECMD_REG, PIT_CMD_CH0 | PIT_ACCESS_LOHIBYTE |
				  PIT_OP_RATEGEN | PIT_MODE_BINARY);
	outb(PIT_CH0_DATA_PORT, count & 0xff);
	outb(PIT_CH0_DATA_PORT, count >> 8);
}

struct timer {
	atomic_uint_least32_t ticks;
	bool _armed;
};

static struct timer __timer;

/*static void timer_on_timeout(void)
{
	if (__timer._armed) {
		__asm__ volatile("int 0x49");
		__timer._armed = false;
	}
}*/

bool timer_tick(void)
{
	if (timer_poll() > 0) {
		if (atomic_fetch_sub_explicit(&__timer.ticks, 1, memory_order_relaxed) == 1) {
			return __timer._armed;
		}
		return false;
	}
	return __timer._armed;
}

u32 timer_poll(void)
{
	return atomic_load_explicit(&__timer.ticks, memory_order_relaxed);
}

u32 timer_set(u32 ticks)
{
	return atomic_exchange_explicit(&__timer.ticks, ticks, memory_order_relaxed);
}

void timer_sleep(u32 millis)
{
	timer_set(millis);
	while (timer_poll())
		__asm__ volatile("hlt");
}

void timer_eoi(void)
{
	lapic_eoi();
}

void timer_init(struct acpi_table *table)
{
	(void)table;
	/*
	 * The PIT has a frequency of about 1.193182 MHz
	 *
	 * So if we want it to tick every millisecond we should set the
	 * frequency divider to:
	 * 1193182/1000=~1193
	 */
	pit_setup(1193);

	struct madt *madt = (struct madt*) acpi_find(table, ACPI_TAG_APIC);
	assert(madt);
	//TODO don't find the first best lapic but the lapic we actually want
	struct madt_lapic *lapic = (struct madt_lapic*) madt_find(madt, MADT_TYPE_LAPIC);
	assert(lapic);

	madt_dump(madt);

	u64 v = IOAPIC_PRIO_NORMAL | IOAPIC_DEST_PHYSICAL |
		IOAPIC_POLARITY_HIGHACTIVE | IOAPIC_TRIGGER_EDGE |
		IOAPIC_PHYSICAL_ID(lapic->lapic_id) | IOAPIC_VECTOR(0x48);

	printk("ioapic: 0x%016llx\n", v);
	ioapic_set_redir(PIT_CH0_IRQ, v);
}

void timer_sched_int(u32 millis)
{
	__timer._armed = true;
	if (!millis) {
		__asm__ volatile("int 0x48");
	} else {
		//TODO shoudl we disable IRQs?
		timer_set(millis);
	}
}

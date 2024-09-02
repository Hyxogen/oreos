#include <kernel/printk.h>
#include <kernel/mmu.h>
#include <kernel/arch/i386/apic.h>
#include <kernel/arch/i386/io.h>
#include <lib/assert.h>

#include <kernel/debug.h>

#define PIC_MASTER_CMD_PORT 0x20
#define PIC_MASTER_DATA_PORT 0x21
#define PIC_SLAVE_CMD_PORT 0xA0
#define PIC_SLAVE_DATA_PORT 0xA1

#define PIC_ICW1_START_INIT 0x10
#define PIC_ICW1_ICW4 0x01
#define PIC_ICW4_8086_MODE 0x01

#define LAPIC_SPURRIOUS_IVR 0xF0

static void outb_and_wait(u16 port, u8 b)
{
	outb(port, b);
	io_wait();
}

/* remaps the PIC IRQ vector numbers to start at the specified offsets */
static void pic_remap(u8 master_offset, u8 slave_offset)
{
	u8 m1, m2;

	/* save masks */
	m1 = inb(PIC_MASTER_DATA_PORT);
	m2 = inb(PIC_SLAVE_DATA_PORT);

	/* ICW1: start init sequence, and indicate that we'll send an ICW4 */
	outb_and_wait(PIC_MASTER_CMD_PORT, PIC_ICW1_START_INIT | PIC_ICW1_ICW4);
	outb_and_wait(PIC_SLAVE_CMD_PORT, PIC_ICW1_START_INIT | PIC_ICW1_ICW4);

	/* set the interrupt vector offset */
	outb_and_wait(PIC_MASTER_DATA_PORT, master_offset);
	outb_and_wait(PIC_SLAVE_DATA_PORT, slave_offset);

	/* ICW3: inform master that there is a slave PIC at IRQ2 (100b) */
	outb_and_wait(PIC_MASTER_DATA_PORT, 4);
	/* inform slave that its identity is PIC number 2 */
	outb_and_wait(PIC_SLAVE_DATA_PORT, 2);


	/* ICW4: set PICs to 8086 mode */
	outb_and_wait(PIC_MASTER_DATA_PORT, PIC_ICW4_8086_MODE);
	outb_and_wait(PIC_SLAVE_DATA_PORT, PIC_ICW4_8086_MODE);

	/* restore masks */
	outb(PIC_MASTER_DATA_PORT, m1);
	outb(PIC_SLAVE_DATA_PORT, m2);
}

static void pic_disable(void)
{
	outb(PIC_MASTER_DATA_PORT, 0xff);
	outb(PIC_SLAVE_DATA_PORT, 0xff);
}

static volatile u32* lapic_get_reg(void *base_addr, u32 reg)
{
	return (volatile u32 *)((u8 *)base_addr + reg);
}

static void lapic_write(void *base_addr, u32 reg, u32 val)
{
	volatile u32 *p = lapic_get_reg(base_addr, reg);
	*p = val;
}

static u32 lapic_read(void *base_addr, u32 reg)
{
	return *lapic_get_reg(base_addr, reg);
}

static void lapic_init(void *base_addr)
{
	u32 val = lapic_read(base_addr, LAPIC_SPURRIOUS_IVR);

	val |= 0x0FF; /* set IRQ number */
	val |= 0x100; /* enable */
	lapic_write(base_addr, LAPIC_SPURRIOUS_IVR, val);
}

static void *__base_addr;

void apic_init(struct madt *madt)
{
	//TODO keep track somewhere of these offsets
	pic_remap(0x20, 0x28);
	printk("remapped PIC offsets to 0x20 and 0x28\n");
	pic_disable();
	printk("disabled PICs\n");

	__base_addr = mmu_map(NULL, madt->lapic_addr, MMU_PAGESIZE, MMU_ADDRSPACE_KERNEL, 0);

	if (__base_addr != MMU_MAP_FAILED)
		lapic_init(__base_addr);
	else
		printk("failed to map lapic addr\n");
}

void lapic_eoi(void)
{
	assert(__base_addr);
	lapic_write(__base_addr, 0xB0, 0);
}

void ioapic_write(void *ioapicaddr, u32 reg, u32 val)
{
	volatile u32 *ioapic = (volatile u32*)ioapicaddr;
	ioapic[0] = reg & 0xff;
	ioapic[4] = val;
}

u32 ioapic_read(void *ioapicaddr, u32 reg)
{
	volatile u32 *ioapic = (volatile u32*)ioapicaddr;
	ioapic[0] = reg & 0xff;
	return ioapic[4];
}

void ioapic_set_redir(void *ioapicaddr, u8 idx, u64 val)
{
	ioapic_write(ioapicaddr, IOAPIC_REG_REDIR(idx), val & 0xffffffff);
	ioapic_write(ioapicaddr, IOAPIC_REG_REDIR(idx) + 1, val >> 32);
}

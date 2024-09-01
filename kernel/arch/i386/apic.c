#include <kernel/printk.h>
#include <kernel/arch/i386/apic.h>
#include <kernel/arch/i386/io.h>

#define PIC_MASTER_CMD_PORT 0x20
#define PIC_MASTER_DATA_PORT 0x21
#define PIC_SLAVE_CMD_PORT 0xA0
#define PIC_SLAVE_DATA_PORT 0xA1

#define PIC_ICW1_START_INIT 0x10
#define PIC_ICW1_ICW4 0x01
#define PIC_ICW4_8086_MODE 0x01

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
	outb_and_wait(PIC_SLAVE_CMD_PORT, PIC_ICW4_8086_MODE);

	/* restore masks */
	outb(PIC_MASTER_DATA_PORT, m1);
	outb(PIC_SLAVE_DATA_PORT, m2);
}

static void pic_disable(void)
{
	outb(PIC_MASTER_DATA_PORT, 0xff);
	outb(PIC_SLAVE_DATA_PORT, 0xff);
}

void apic_init(struct madt *madt)
{
	pic_remap(0x20, 0x28);
	printk("remapped PIC offsets to 0x20 and 0x28\n");
	pic_disable();
	printk("disabled PICs\n");
}

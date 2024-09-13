#include <kernel/ps2.h>
#include <kernel/arch/i386/io.h>
#include <kernel/arch/i386/apic.h>
#include <kernel/irq.h>

#define PS2_DATA_PORT 0x60
#define PS2_STATUS_PORT 0x64

#define PS2_PORT1_IRQN 0x01

void ps2_send(u8 b)
{
	outb(PS2_DATA_PORT, b);
}

void ps2_send_cmd(u8 b)
{
	outb(PS2_STATUS_PORT, b);
}

u8 ps2_recv(void)
{
	return inb(PS2_DATA_PORT);
}

bool ps2_canrecv(void)
{
	u8 b = inb(PS2_STATUS_PORT);
	return b & 0b00000001;
}

bool ps2_cansend(void)
{
	u8 b = inb(PS2_STATUS_PORT);
	return !(b & 0b00000010);
}

i16 ps2_setup_irq(void)
{
	i16 irqn = irq_get_free_irq();
	if (irqn < 0)
		return -1;
	//TODO don't hardcode ioapic physical id
	u64 redir =
	    IOAPIC_REDIR(IOAPIC_PRIO_NORMAL, IOAPIC_DEST_PHYSICAL,
			 IOAPIC_POLARITY_HIGHACTIVE, IOAPIC_TRIGGER_EDGE,
			 IOAPIC_PHYSICAL_ID(0), IOAPIC_VECTOR(irqn));

	if (!ioapic_set_redir(PS2_PORT1_IRQN, redir))
		return -1;
	return irqn;
}

void ps2_eoi(void)
{
	lapic_eoi();
}

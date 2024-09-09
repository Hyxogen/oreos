#include <kernel/ps2.h>
#include <kernel/arch/i386/io.h>

#define PS2_DATA_PORT 0x60
#define PS2_STATUS_PORT 0x64

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

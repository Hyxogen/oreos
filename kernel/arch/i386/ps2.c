#include <kernel/ps2.h>
#include <kernel/arch/i386/io.h>

#define PS2_DATA_PORT 0x60
#define PS2_STATUS_PORT 0x64

static void sendb(u16 port, u8 b)
{
	outb(port, b);
}

static u8 recvb(u16 port)
{
	return inb(port);
}

void ps2_send(u8 b)
{
	sendb(PS2_DATA_PORT, b);
}

void ps2_send_cmd(u8 b)
{
	sendb(PS2_STATUS_PORT, b);
}

u8 ps2_recv(void)
{
	return recvb(PS2_DATA_PORT);
}

bool ps2_canrecv(void)
{
	u8 b = recvb(PS2_STATUS_PORT);
	return b & 0b00000001;
}

bool ps2_cansend(void)
{
	u8 b = recvb(PS2_STATUS_PORT);
	return !(b & 0b00000010);
}

#include <kernel/ps2.h>

#define PS2_DATA_PORT 0x60
#define PS2_STATUS_PORT 0x64

static void sendb(u16 port, u8 b)
{
	__asm__ ("mov %%dx, %0\n\t"
		 "mov %%al, %1\n\t"\
		 "out %%dx, %%al"
		 : : "r"(port), "r"(b) : "dx", "al");
}

static u8 recvb(u16 port)
{
	u8 b;

	__asm__ ("mov %%dx, %1\n\t"
		 "in %%al, %%dx\n\t"
		 "mov %0, %%al"
		 : "=r"(b) : "r"(port) : "dx", "al");

	return b;
}

void ps2_send(u8 b)
{
	sendb(PS2_DATA_PORT, b);
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

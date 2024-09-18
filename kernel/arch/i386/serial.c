#include <kernel/serial.h>
#include <kernel/kernel.h>
#include <kernel/arch/i386/io.h>

static inline u16 com_to_port(int com)
{
	switch (com) {
	case 1:
		return 0x3F8;
	}
	panic("unimplemented");
}

void ser_write(int port, int reg, u8 val)
{
	outb(com_to_port(port) + reg, val);
}

u8 ser_read(int port, int reg)
{
	return inb(com_to_port(port) + reg);
}

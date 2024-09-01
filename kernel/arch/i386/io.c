#include <kernel/arch/i386/io.h>

void outb(u16 port, u8 b)
{
	__asm__ volatile("out %w0, %b1" : : "Nd"(port), "a"(b) : "memory");
}

u8 inb(u16 port)
{
	u8 b;

	__asm__ volatile("in %b0, %w1" : "=a"(b) : "Nd"(port) : "memory");

	return b;
}

void io_wait(void)
{
	/* This waits a very small amount (1-4 microseconds by doing an IO
	 * operation to an unused port.
	 *
	 * Apparently Linux uses 0x80, which is:
	 * "often used during POST to log informatnoi to the motherboards's hex
	 * display but almost always unused after boot"
	 */
	outb(0x80, 0);
}

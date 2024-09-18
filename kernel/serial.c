#include <stdbool.h>
#include <stddef.h>
#include <kernel/serial.h>

#include <kernel/kernel.h>
#include <kernel/libc/assert.h>

int width, height;

// from https://www.sci.muni.cz/docs/pc/serport.txt
static int ser_detect_uart(int port)
{
	/* check if UART present */
	u8 old = ser_read(port, SERIAL_REG_MCR);

	ser_write(port, SERIAL_REG_MCR, SERIAL_MCR_LOOPBACK_BIT);

	/* when LOOP bit set, the upper 4 bits of the MSR will mirror the status
	 * lines of the MCR */
	u8 x = ser_read(port, SERIAL_REG_MSR);
	if (x & 0xf0)
		return SERIAL_UART_NONE;

	ser_write(port, SERIAL_REG_MCR, SERIAL_MCR_LOOPBACK_BIT | 0x0f);
	x = ser_read(port, SERIAL_REG_MSR);
	if ((x & 0xf0) != 0xf0)
		return SERIAL_UART_NONE;

	ser_write(port, SERIAL_REG_MCR, old);

	/* check for scratch register */
	old = ser_read(port, SERIAL_REG_SCRATCH);

	u8 magic = 0x55;
	ser_write(port, SERIAL_REG_SCRATCH, magic);
	if (ser_read(port, SERIAL_REG_SCRATCH) != magic)
		return SERIAL_UART_8250;

	magic = 0xaa;
	ser_write(port, SERIAL_REG_SCRATCH, magic);
	if (ser_read(port, SERIAL_REG_SCRATCH) != magic)
		return SERIAL_UART_8250;

	ser_write(port, SERIAL_REG_SCRATCH, old);

	/* check for fifo */
	ser_write(port, SERIAL_REG_FCR, SERIAL_FCR_ENABLE_BIT);

	x = ser_read(port, SERIAL_REG_IIR);

	/* apparently some old fashioned software relies on this */
	ser_write(port, SERIAL_REG_FCR, 0);

	/*
	 * 0 0 -> 2
	 * 0 1 -> 2
	 * 1 0 -> 3
	 * 1 1 -> 4
	 *
	 */
	//TODO
	return -1;
}

static bool ser_can_transmit(int port)
{
	return ser_read(port, SERIAL_REG_LSR) & SERIAL_LSR_TEMT;
}

static void ser_polled_putc(int port, u8 byte)
{
	while (!ser_can_transmit(port))
		continue;
	ser_write(port, SERIAL_REG_WRITE, byte);
}

void ser_putc(int com, u8 byte)
{
	//TODO irqs
	ser_polled_putc(com, byte);
	if (byte == '\n')
		ser_polled_putc(com, '\r');
}

static int ser_set_baud(int port, int baud)
{
	u8 old_lcr = ser_read(port, SERIAL_REG_LCR);
	/* enable dlab to set buad rate divisor */
	ser_write(port, SERIAL_REG_LCR, SERIAL_LCR_DLAB_BIT);

	int div = SERIAL_CLOCK_SPEED / baud;
	if (!div || div > (u8) -1)
		return -1;

	/* set baud rate divisor */
	/* TODO don't assume little endian */
	ser_write(port, SERIAL_REG_BAUDDIV_LO, div & 0xff);
	ser_write(port, SERIAL_REG_BAUDDIV_HI, (div >> 8) & 0xff);

	ser_write(port, SERIAL_REG_LCR, old_lcr);
	return 0;
}

static int init_serial_port(int port, int baud)
{
	/* disable interrups */
	ser_write(port, SERIAL_REG_IRQ_ENABLE, 0);

	ser_set_baud(port, baud);

	ser_write(port, SERIAL_REG_LCR, SERIAL_LCR_CHARLEN_8 | SERIAL_LCR_STOPBIT_BIT);

	ser_write(port, SERIAL_REG_FCR,
		  SERIAL_FCR_ENABLE_BIT | SERIAL_FCR_IRQLEVEL_14BYTE |
		      SERIAL_FCR_CLEAR_RX_BIT | SERIAL_FCR_CLEAR_TX_BIT);

	ser_write(port, SERIAL_REG_MCR, SERIAL_MCR_DTR | SERIAL_MCR_RTS | SERIAL_MCR_OUT1 | SERIAL_MCR_OUT2);
	return 0;
}

static int init_early_serial_port(int port, int baud)
{
	if (ser_set_baud(port, baud))
		return -1;
	ser_write(port, SERIAL_REG_MCR, 0);

	return 0;
}

void init_early_serial(void)
{
	assert(!init_early_serial_port(SERIAL_COM1, 38400));
}

void init_serial(void)
{
	//TODO
	panic("TODO\n");
	assert(!init_serial_port(1, 38400));
}

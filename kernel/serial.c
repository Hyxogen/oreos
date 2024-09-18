#include <stdbool.h>
#include <stddef.h>
#include <kernel/serial.h>
#include <kernel/endian.h>

#include <kernel/libc/string.h>

#include <kernel/kernel.h>
#include <kernel/libc/assert.h>

#define SERIAL_BAUD_RATE 38400

// from https://www.sci.muni.cz/docs/pc/serport.txt
static int ser_detect_uart(int port)
{
	/* check if UART present */
	u8 old = ser_read(port, SERIAL_REG_MCR);

	ser_write(port, SERIAL_REG_MCR, SERIAL_MCR_LOOPBACK_BIT);

	/* when LOOP bit set, the upper 4 bits of the MSR will mirror the status
	 * lines of the MCR, we check if this happens */
	u8 x = ser_read(port, SERIAL_REG_MSR);
	if (x & 0xf0)
		return SERIAL_UART_NONE;

	ser_write(port, SERIAL_REG_MCR, SERIAL_MCR_LOOPBACK_BIT | 0x0f);
	x = ser_read(port, SERIAL_REG_MSR);
	if ((x & 0xf0) != 0xf0)
		return SERIAL_UART_NONE;

	ser_write(port, SERIAL_REG_MCR, old);

	/* check for working scratch register */
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

	/* check for working fifo */
	ser_write(port, SERIAL_REG_FCR, SERIAL_FCR_ENABLE_BIT);

	x = ser_read(port, SERIAL_REG_IIR);

	/* apparently some old fashioned software relies on this */
	ser_write(port, SERIAL_REG_FCR, 0);

	switch (x & SERIAL_IIR_FIFO_MASK) {
	case SERIAL_IIR_FIFO_UNUSABLE | SERIAL_IIR_FIFO_ENABLED:
		return SERIAL_UART_16450A;
	case SERIAL_IIR_FIFO_NONE:
	case SERIAL_IIR_FIFO_UNUSABLE:
		return SERIAL_UART_16450_OR_8250_SCRATCH;
	case SERIAL_IIR_FIFO_ENABLED:
		return SERIAL_UART_16450;
	}
	unreachable();
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

void ser_writestr(int com, const char *str)
{
	while (*str) {
		ser_putc(com, *str++);
	}
}

static int ser_set_baud(int port, u32 baud)
{
	u8 saved_lcr = ser_read(port, SERIAL_REG_LCR);

	/* enable dlab to set buad rate divisor */
	ser_write(port, SERIAL_REG_LCR, SERIAL_LCR_DLAB_BIT);

	u32 div = SERIAL_CLOCK_SPEED / baud;
	if (!div || div > (u8) -1)
		return -1;

	/* set baud rate divisor */
	div = le_u16(div);

	ser_write(port, SERIAL_REG_BAUDDIV_LO, div & 0xff);
	ser_write(port, SERIAL_REG_BAUDDIV_HI, (div >> 8) & 0xff);

	ser_write(port, SERIAL_REG_LCR, saved_lcr);
	return 0;
}

static int init_serial_port(int port, u32 baud)
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

static int init_early_serial_port(int port, u32 baud)
{
	int uart = ser_detect_uart(port);
	if (uart == SERIAL_UART_NONE)
		return -1; /* no uart present */

	if (ser_set_baud(port, baud))
		return -1;
	ser_write(port, SERIAL_REG_MCR, 0);

	char str[2] = { '0' + uart, '\0' };
	ser_writestr(port, "uart: ");
	ser_writestr(port, str);
	ser_putc(port, '\n');

	return 0;
}

void init_early_serial(void)
{
	assert(!init_early_serial_port(SERIAL_COM1, SERIAL_BAUD_RATE));
}

void init_serial(void)
{
	/* TODO actually handle the interrupts for reading
	 *
	 * At the time of writing this, I don't need to read from serial, so I
	 * didn't implement it, but it is probably a good idea to have at some
	 * point.
	 *
	 * When enabling interrupts, is it actually interesting to wait for the
	 * serial port to become ready to receive before sending? It (probably)
	 * has a FIFO buffer already, so I would only really be duplicating
	 * that.
	 *
	 * On the other hand, I guess that IO operations are slow, and doing
	 * them all at ones might have a benifit?
	 *
	 */
	assert(!init_serial_port(SERIAL_COM1, SERIAL_BAUD_RATE));
}

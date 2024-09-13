#include <stdbool.h>
#include <stdatomic.h>
#include <kernel/platform.h>
#include <kernel/printk.h>
#include <kernel/debug.h>
#include <kernel/irq.h>

static void redraw_term(void)
{
	struct term *term = term_get_primary();

	term_redraw(term);
	printk_set_sink(term);
}

void oops(const char *fmt, ...)
{
	redraw_term();

	printk("OOPS! Something went wrong!\n");

	va_list args;
	va_start(args, fmt);

	vprintk(fmt, args);
	va_end(args);
}

void panic(const char *fmt, ...)
{
	static atomic_bool panicking = false;
	disable_irqs();

	if (atomic_exchange(&panicking, true))
		goto halt; /* we are already panicing */

	redraw_term();

	printk("Oops! A kernel panic ocurred :/\n");

	va_list args;
	va_start(args, fmt);

	vprintk(fmt, args);
	va_end(args);

	printk("\n");

halt:
	BOCHS_BREAK;
	idle();
}

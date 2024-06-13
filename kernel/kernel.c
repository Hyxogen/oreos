#include <stdbool.h>

#include <kernel/framebuf.h>
#include <kernel/tty.h>
#include <kernel/ps2.h>
#include <kernel/kernel.h>

void kernel_main(void)
{
	struct term *term = term_get_primary();

	printk_set_sink(term);
	printk("\033[31m42\n");

	ps2_init();

	while (1) {
		enum keycode k = ps2_getkey_timeout();

		if (k == KEYCODE_1) {
			printk("\033[2J");
		} else if ((int) k > 0) {
			printk("%c", kc_toascii(k));
		}
	}
}

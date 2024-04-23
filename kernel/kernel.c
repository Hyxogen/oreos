#include <kernel/tty.h>
#include <kernel/ps2.h>
#include <kernel/kernel.h>

void kernel_main(void)
{
	ps2_init();

	printk("\033[45m%hx\n", (unsigned short) 42);

	while (1) {
		enum keycode k = ps2_getkey_timeout();

		if (k == KEYCODE_1) {
			printk("\033[2J");
		} else if ((int) k > 0) {
			printk("%c", kc_toascii(k));
		}
	}
	//term_print("Hello\nWorld!");
}

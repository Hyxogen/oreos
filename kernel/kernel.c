#include <kernel/tty.h>
#include <kernel/ps2.h>
#include <kernel/kernel.h>

void kernel_main(void)
{
	ps2_init();

	printk("%hx\n", (unsigned short) 42);

	while (1) {
		enum keycode k = ps2_getkey_timeout();

		if ((int) k > 0) {
			printk("%c", kc_toascii(k));
		}
	}
	//term_print("Hello\nWorld!");
}

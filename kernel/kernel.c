#include <stdbool.h>

#include <kernel/framebuf.h>
#include <kernel/kernel.h>
#include <kernel/ps2.h>
#include <kernel/tty.h>

void gdt_init(void);

void kernel_main(void)
{
	struct term *term = term_get_primary();
	static struct term alt;

	term_init(&alt, fb_get_primary());
	printk_set_sink(term);
	printk("\033[31m42\n");

	ps2_init();
	gdt_init();

	while (1) {
		enum keycode k = ps2_getkey_timeout();

		if (k == KEYCODE_F1) {
			printk_set_sink(term);
			term_redraw(term);
		} else if (k == KEYCODE_F2) {
			printk_set_sink(&alt);
			term_redraw(&alt);
		}

		if ((int)k > 0) {
			printk("%c", kc_toascii(k));
		}
	}
}

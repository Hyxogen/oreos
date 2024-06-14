#include <stdbool.h>
#include <stdarg.h>

#include <kernel/framebuf.h>
#include <kernel/kernel.h>
#include <kernel/ps2.h>
#include <kernel/tty.h>
#include <libc/string.h>

void gdt_init(void);

static void exec_cmd(const char *str)
{
	if (!strcmp(str, "panic")) {
		panic("explicit panic\n");
	} else if (!strcmp(str, "reboot")) {
		reset();
	} else if (!strcmp(str, "halt")) {
		halt();
	} else if (!strncmp(str, "dumpstack ", 10)) {
	}
}

void panic(const char *fmt, ...)
{
	struct term *term = term_get_primary();

	term_redraw(term);
	printk_set_sink(term);

	va_list args;
	va_start(args, fmt);

	vprintk(fmt, args);
	va_end(args);

	printk("Oops! A kernel panic ocurred :/\n");
	halt();
}

void kernel_main(void)
{
	struct term *term = term_get_primary();
	static struct term alt;

	term_init(&alt, fb_get_primary());
	printk_set_sink(term);
	printk("\033[31m42\n");

	ps2_init();
	gdt_init();

	bool shell = false;
	char buf[80];
	unsigned i = 0;

	buf[0] = 0;

	while (1) {
		enum keycode k = ps2_getkey_timeout();

		if (k == KEYCODE_F1) {
			printk_set_sink(term);
			term_redraw(term);
			shell = false;
		} else if (k == KEYCODE_F2) {
			printk_set_sink(&alt);
			term_redraw(&alt);
			shell = true;
		}

		if ((int)k > 0) {
			int ch = kc_toascii(k);

			printk("%c", ch);

			if (shell) {
				if (i >= sizeof(buf) - 1) {
					printk("command too long\n");
					i = 0;
					buf[0] = '\0';
				} else if (ch) {
					if ((char) ch == '\n') {
						exec_cmd(buf);
						buf[0] = '\0';
						i = 0;
					} else {
						buf[i++] = (char) ch;
						buf[i] = '\0';
					}
				}
			}
		}
	}
}

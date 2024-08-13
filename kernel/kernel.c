#include <stdarg.h>
#include <stdbool.h>

#include <kernel/framebuf.h>
#include <kernel/kernel.h>
#include <kernel/printk.h>
#include <kernel/ps2.h>
#include <kernel/tty.h>
#include <kernel/mman.h>
#include <lib/ctype.h>
#include <lib/kstrtox.h>
#include <lib/string.h>

void gdt_init(void);

static void print_hexdump(const void *p, size_t n)
{
	const unsigned char *s = (const unsigned char *)p;
	const unsigned bytes_per_row = 16;

	for (size_t offset = 0; offset < n; offset += 16) {
		if (offset)
			printk("\n");

		printk("%p:", (void *)(s + offset));

		for (size_t i = 0; i < bytes_per_row; ++i) {
			char byte = s[offset + i];

			if ((i % 2) == 0)
				printk(" ");

			if (offset + i >= n) {
				printk("  ");
			} else {
				if (isprint(byte))
					printk("\033[32m");
				else
					printk("\033[33m");
				printk("%02hhx", s[offset + i]);
				printk("\033[m");
			}
		}

		printk("  ");

		for (size_t i = 0; i < 16 && offset + i < n; ++i) {
			if (isprint(s[offset + i]))
				printk("\033[32m%c", s[offset + i]);
			else
				printk("\033[33m.");
			printk("\033[m");
		}
	}
	printk("\n");
}

extern u8 _stack_top;

static void exec_cmd(const char *str)
{
	if (!strcmp(str, "panic")) {
		panic("explicit panic\n");
	} else if (!strcmp(str, "reboot")) {
		reset();
	} else if (!strcmp(str, "halt")) {
		halt();
	} else if (!strncmp(str, "dumpstack", 9)) {
		if (!str[9]) {
			printk("usage: dumpstack <offset> [nbytes]\n");
			return;
		}

		char *end;
		unsigned long offset;

		enum lib_error res = kstrtoul(&str[10], &end, 0, &offset);
		if (res != LIB_OK) {
			kperror("dumpstack", res);
			return;
		}

		while (isspace(*end))
			end++;

		unsigned long n = offset;
		if (*end) {
			if ((res = kstrtoul(end, NULL, 0, &n))) {
				kperror("dumpstack", res);
				return;
			}
		}

		print_hexdump((char *)&_stack_top - offset, n);
	} else {
		printk("unknown command: '%s'\n", str);
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

	mman_dump();

	bool shell = false;
	char buf[80];
	unsigned i = 0;

	buf[0] = 0;

	printk("main: %p\n", (void*)kernel_main);

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
					if ((char)ch == '\n') {
						exec_cmd(buf);
						buf[0] = '\0';
						i = 0;
					} else {
						buf[i++] = (char)ch;
						buf[i] = '\0';
					}
				}
			}
		}
	}
}

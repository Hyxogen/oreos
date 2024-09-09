#include <kernel/kernel.h>
#include <kernel/platform.h>
#include <kernel/printk.h>
#include <kernel/debug.h>
#include <kernel/libc/string.h>
#include <kernel/libc/kstrtox.h>
#include <kernel/libc/ctype.h>
#include <kernel/ps2.h>

static void exec_cmd(const char *str)
{
	if (!strcmp(str, "panic")) {
		panic("explicit panic\n");
	} else if (!strcmp(str, "reboot")) {
		reset();
	} else if (!strcmp(str, "halt")) {
		idle();
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

void start_shell(void)
{
	printk("shell started\n");
	char buf[80];
	unsigned i = 0;

	buf[0] = 0;

	while (1) {
		enum keycode k = ps2_getkey_timeout();

		if ((int)k > 0) {
			int ch = kc_toascii(k);

			printk("%c", ch);

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

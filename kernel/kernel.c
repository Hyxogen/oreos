#include <stdarg.h>
#include <stdbool.h>

#include <kernel/debug.h>
#include <kernel/framebuf.h>
#include <kernel/kernel.h>
#include <kernel/printk.h>
#include <kernel/ps2.h>
#include <kernel/tty.h>
#include <kernel/mmu.h>
#include <kernel/libc/ctype.h>
#include <kernel/libc/kstrtox.h>
#include <kernel/libc/string.h>
#include <kernel/malloc/malloc.h>
#include <kernel/timer.h>
#include <kernel/platform.h>
#include <kernel/sched.h>
#include <kernel/libc/assert.h>

#include <kernel/acpi/acpi.h>

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

void init_paging(void);
void init_segments(void);
void init_framebuf(struct mb2_info *info);

static struct term *term = NULL;

void init_consoles(struct mb2_info *info)
{
	init_framebuf(info);

	term = term_get_primary();
	printk_set_sink(term);
	init_ps2();
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

void read_acpi(struct acpi_table *table, struct mb2_info *info)
{
	struct mb2_acpi_new *acpi = (void*) mb2_find(info, MB2_TAG_TYPE_ACPI_NEW);

	if (!acpi) {
		printk("could not find ACPI tag\n");

		struct mb2_acpi_old *old = (void*) mb2_find(info, MB2_TAG_TYPE_ACPI_OLD);
		if (old) {
			printk("did find legacy\n");

			acpi_read(table, old->rsdp);
		}
		return;
	}


	acpi_read(table, acpi->xsdp);
}

void init_irq_handler(struct acpi_table *table);

void dummy(void)
{
	while (1) {
		printk("dummy\n");
		__asm__ volatile("hlt");
	}
}

void kernel_main(struct mb2_info *info)
{
	struct acpi_table table = {};

	init_paging();
	init_segments();
	init_mmu(info);
	init_consoles(info);
	read_acpi(&table, info);
	init_irq_handler(&table);
	init_timer(&table);
	//init_printk(); TODO
	
	printk("a\n");
	//timer_sleep(2000);
	printk("b\n");

	mmu_unmap(info, info->total_size); // we're done with multiboot, free it

	printk("done!\n");
	init_sched();

	struct cpu_state *state = proc_create(start_shell);
	assert(state);
	struct cpu_state *dummystate = proc_create(dummy);
	assert(dummystate);

	assert(!sched_proc(state));
	assert(!sched_proc(dummystate));

	sched_start();
}

#include <stddef.h>
#include <kernel/printk.h>
#include <kernel/libc/ctype.h>

void print_hexdump(const void *p, size_t n)
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


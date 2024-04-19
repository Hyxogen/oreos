#include <kernel/tty.h>

size_t strlen(const char *str)
{
	const char *beg = str;

	while (*str)
		++str;
	return str - beg;
}

void kernel_main(void)
{
	//term_putchar(1, 1, 'x', COLOR_WHITE, COLOR_BLACK);
	term_print("Hello\nWorld!");
}

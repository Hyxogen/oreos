#include <kernel/tty.h>

void kernel_main(void)
{
	//term_putchar(1, 1, 'x', COLOR_WHITE, COLOR_BLACK);
	term_print("Hello\nWorld!");
}

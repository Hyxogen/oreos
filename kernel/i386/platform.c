#include <kernel/kernel.h>
#include <kernel/tty.h> //TTY will always be available

__attribute__ ((noreturn)) void _idle();

void panic(const char *fmt, ...)
{
	term_print(fmt);
	term_print("Oops! A kernel panic ocurred :/");
	_idle();
}

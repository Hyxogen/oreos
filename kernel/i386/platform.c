#include <kernel/kernel.h>
#include <kernel/tty.h> //TTY will always be available

__attribute__ ((noreturn)) void _idle();

void panic(const char *fmt, ...)
{
	struct term *term = term_get_primary();

	term_print(term, fmt);
	term_print(term, "Oops! A kernel panic ocurred :/");
	_idle();
}

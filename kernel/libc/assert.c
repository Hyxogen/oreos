#include <kernel/kernel.h>
#include <kernel/printk.h>
#include <kernel/libc/stdlib.h>

#ifndef NDEBUG
void __assert_impl(int c, const char *pred, const char *file, const char *func,
		   int line)
{
	if (!c) {
		printk("%s:%i: %s: Assertion '%s` failed.\n", file, line, func,
		       pred);
		abort();
	}
}
#endif

[[noreturn]] void abort(void)
{
	panic("aborted");
}

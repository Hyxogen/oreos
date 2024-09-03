#include <kernel/libc/error.h>
#include <kernel/printk.h>

const char *kstrerr(enum lib_error err)
{
	switch (err) {
	case LIB_OK:
		return "No error";
	case LIB_ERANGE:
		return "Numerical result out of range";
	case LIB_EINVAL:
	default:
		return "Invalid argument";
	}
}

void kperror(const char *s, enum lib_error err)
{
	if (s)
		printk("%s: ", s);
	printk("%s\n", kstrerr(err));
}

#include <kernel/tty.h>
#include <kernel/kernel.h>
#include <limits.h>

void kernel_main(void)
{
	//printk("%i %i %i\n", 0, -1, 1);
	//printk("%.5i %.5i %.5i\n", 0, -1, 1);
	//printk("%10.5i %*.5i %10.5i\n", 0, 10, -1, 1);
	printk("\"%-10.5i\"\n", 1);
	//term_print("Hello\nWorld!");
}

#include <kernel/kernel.h>
#include <kernel/printk.h>

#define DUMP_REGISTER(name)                                        \
	do {                                                       \
		u32 __val;                                         \
		__asm__ volatile("mov %0, %%" name : "=r"(__val)); \
		printk("%s = 0x%08lx\n", name, __val);             \
	} while (0)

void dump_registers(void)
{
	DUMP_REGISTER("eax");
	DUMP_REGISTER("ebx");
	DUMP_REGISTER("ecx");
	DUMP_REGISTER("edx");
	DUMP_REGISTER("esi");
	DUMP_REGISTER("edi");
	DUMP_REGISTER("esp");
	DUMP_REGISTER("ebp");
}

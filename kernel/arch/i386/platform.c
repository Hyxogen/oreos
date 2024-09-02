#include <kernel/kernel.h>
#include <kernel/tty.h> //TTY will always be available
#include <kernel/ps2.h>
#include <kernel/printk.h>
__attribute__((noreturn)) void _idle();

void reset(void)
{
	while (!ps2_cansend())
		continue;
	ps2_send_cmd(PS2_CMD_RESET);
	_idle();
}

void halt(void)
{
	_idle();
}

void dump_stacktrace_from(const void *ebpp)
{
	const u32* ebp = ebpp;
	unsigned level = 0;

	while (ebp) {
		printk("%03d: 0x%08lx\n", level, *(ebp + 1));
		ebp = (const u32*) *ebp;
		level += 1;
	}
}

void dump_stacktrace(void)
{
	u32 *ebp;
	__asm__ volatile("mov %0,%%ebp" : "=r"(ebp));
	dump_stacktrace_from(ebp);
}

#define DUMP_REGISTER(reg)                                         \
	do {                                                       \
		u32 __val;                                         \
		__asm__ volatile("mov %0," reg : "=r"(__val));     \
		printk("%s: 0x%08lx (%lu)\n", reg, __val, __val); \
	} while (0)

void dump_registers(void)
{
	DUMP_REGISTER("eax");
	DUMP_REGISTER("ebx");
	DUMP_REGISTER("ecx");
	DUMP_REGISTER("edx");
	DUMP_REGISTER("esi");
	DUMP_REGISTER("edi");
	DUMP_REGISTER("esi");
	DUMP_REGISTER("ebp");
}

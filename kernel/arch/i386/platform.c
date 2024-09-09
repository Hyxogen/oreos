#include <kernel/kernel.h>
#include <kernel/tty.h> //TTY will always be available
#include <kernel/ps2.h>
#include <kernel/printk.h>
#include <kernel/platform.h>
#include <kernel/arch/i386/platform.h>

void reset(void)
{
	while (!ps2_cansend())
		continue;
	ps2_send_cmd(PS2_CMD_RESET);
	idle();
}

void halt(void)
{
	__asm__ volatile("hlt");
}

static void dump_stacktrace_ebp(const void *ebpp)
{
	const u32* ebp = ebpp;
	unsigned level = 0;

	while (ebp) {
		printk("%03d: 0x%08lx\n", level, *(ebp + 1));
		ebp = (const u32*) *ebp;
		level += 1;
	}
}

void dump_stacktrace_at(const struct cpu_state *state)
{
	dump_stacktrace_ebp((void*) (uintptr_t)state->ebp);
}

void dump_stacktrace(void)
{
	u32 *ebp;
	__asm__ volatile("mov %0,%%ebp" : "=r"(ebp));
	dump_stacktrace_ebp(ebp);
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

void disable_irqs(void)
{
	__asm__ volatile("cli");
}

void enable_irqs(void)
{
	__asm__ volatile("sti");
}

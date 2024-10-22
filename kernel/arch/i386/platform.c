#include <kernel/kernel.h>
#include <kernel/tty.h> //TTY will always be available
#include <kernel/ps2.h>
#include <kernel/printk.h>
#include <kernel/platform.h>
#include <kernel/arch/i386/platform.h>
#include <kernel/arch/i386/io.h>

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

void dump_state(const struct cpu_state *state)
{
	DUMP_REGISTER("cr0");
	DUMP_REGISTER("cr2");
	DUMP_REGISTER("cr3");
	DUMP_REGISTER("cr4");

	printk("vec_num: 0x%08lx ", state->vec_num);
	printk("err_code: 0x%08lx\n", state->err_code);

	printk("edi: 0x%08lx ", state->edi);
	printk("esi: 0x%08lx ", state->esi);
	printk("ebp: 0x%08lx\n", state->ebp);
	printk("esp: 0x%08lx ", state->esp);
	printk("ebx: 0x%08lx ", state->ebx);
	printk("edx: 0x%08lx\n", state->edx);
	printk("ecx: 0x%08lx ", state->ecx);
	printk("eax: 0x%08lx ", state->eax);
	printk("eip: 0x%08lx\n", state->eip);
	printk("cs: 0x%04hx\n", state->cs);

}

void disable_irqs(void)
{
	__asm__ volatile("cli");
}

void enable_irqs(void)
{
	__asm__ volatile("sti");
}

void short_wait(void)
{
	io_wait();
}

bool is_from_userspace(const struct cpu_state *state)
{
	return state->eflags.iopl == 3;
}

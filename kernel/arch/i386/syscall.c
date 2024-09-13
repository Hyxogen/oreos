#include <stddef.h>
#include <kernel/kernel.h>
#include <kernel/types.h>
#include <kernel/errno.h>
#include <kernel/syscall.h>
#include <kernel/arch/i386/platform.h>

static i32 syscall_stub(void)
{
	return -ENOTSUP;
}

static i32 (*_syscall_table[])() = {
	syscall_stub, /* doesn't exist */
	syscall_stub, /* _exit */
	syscall_stub, /* fork */
	read, /* read */
	write, /* write */
};

int do_syscall(struct cpu_state *state)
{
	size_t idx = state->eax;

	if (!idx || idx > ARRAY_SIZE(_syscall_table))
		return -1;

	// syscall argument order: eax, ebx, ecx, edx, esi, edi, and ebp
	_syscall_table[idx](state->ebx, state->ecx, state->edx, state->esi, state->edi, state->ebp);
	return 0;
}

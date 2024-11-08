#include <kernel/syscall.h>
#include <kernel/sched.h>
#include <kernel/libc/assert.h>

i32 syscall_exit(struct cpu_state *state, int exit_code)
{
	(void) state;
	sched_do_kill(exit_code);
}

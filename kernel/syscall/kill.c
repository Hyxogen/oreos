#include <kernel/syscall.h>
#include <kernel/sched.h>
#include <kernel/signal.h>
#include <kernel/errno.h>

i32 syscall_kill(struct cpu_state *state, int pid, int sig)
{
	(void)state;
	if (!is_valid_signal(sig))
		return -EINVAL;

	return sched_signal(pid, sig);
}

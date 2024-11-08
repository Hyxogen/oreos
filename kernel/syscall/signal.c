#include <kernel/syscall.h>
#include <kernel/sched.h>
#include <kernel/errno.h>
#include <kernel/signal.h>

i32 syscall_signal(struct cpu_state *state, int signum, void (*handler)(int))
{
	(void) state;
	if (!is_valid_signal(signum))
		return -EINVAL;

	if (signum == SIGKILL || signum == SIGSTOP)
		return 0;

	struct process *proc = sched_get_current_proc();

	/* TODO atomic load? */
	proc->signal_handlers[signum] = handler;

	return 0;
}

#include <kernel/syscall.h>
#include <kernel/sched.h>
#include <kernel/errno.h>

i32 syscall_signal(struct cpu_state *state, int signum, void (*handler)(int))
{
	(void) state;
	struct process *proc = sched_get_current_proc();
	/* TODO block signal handlers for uncatchable signals */
	if (signum >= 32)
		return -EINVAL;

	/* TODO atomic load? */
	proc->signal_handlers[signum] = handler;

	proc_release(proc);
	/* TODO return previous handler */
	return 0;
}

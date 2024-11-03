#include <kernel/syscall.h>
#include <kernel/sched.h>

i32 syscall_kill(struct cpu_state *state, int pid, int sig)
{
	(void)state;
	/* TODO find process by pid */
	struct process *proc = sched_get_current_proc();

	sched_signal(proc, sig);

	proc_release(proc);
	return 0;
}

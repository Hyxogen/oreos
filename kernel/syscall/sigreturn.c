#include <kernel/syscall.h>
#include <kernel/sched.h>
#include <kernel/signal.h>

i32 syscall_sigreturn(struct cpu_state *state)
{
	struct process *proc = sched_get_current_proc();

	if (proc_do_sigreturn(proc, state))
		sched_signal(proc, SIGSEGV);
	proc_release(proc);
	sched_yield(state);
	/* we cannot return, as it will try to set the return value of the syscall */
}

#include <kernel/syscall.h>
#include <kernel/sched.h>

i32 syscall_getpid(void)
{
	struct process *proc = sched_get_current_proc();

	int res = proc->pid;
	return res;
}

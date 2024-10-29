#include <kernel/syscall.h>
#include <kernel/sched.h>
#include <kernel/errno.h>

i32 syscall_fork(void)
{
	return -ENOTSUP;

	struct process *proc = sched_get_current_proc();
	struct process *child = proc_clone(proc);

	i32 res = -1;
	if (child) {
		if (!sched_schedule(child))
			res = child->pid;
		proc_release(child);
	}

	proc_release(proc);
	return res;
}

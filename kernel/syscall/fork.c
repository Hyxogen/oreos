#include <kernel/syscall.h>
#include <kernel/sched.h>
#include <kernel/errno.h>
#include <kernel/mmu.h>

/*
 * het lijkt erop dat er niet goed word wordt geturned uit irq handlers, de
 * context word geupdate naar een nieuwe state, maar komt niet goed terug
 */
i32 syscall_fork(struct cpu_state *state)
{
	struct process *proc = sched_get_current_proc();
	struct process *child = proc_clone(proc, state);

	if (!child)
		return -ENOMEM;

	proc_set_syscall_ret(child->context, 0);

	int res = sched_schedule(child);

	if (!res)
		res = child->pid;

	proc_release(proc);
	proc_release(child);

	/* pages are now CoW */
	mmu_invalidate_user();

	return res;
}

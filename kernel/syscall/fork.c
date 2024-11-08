#include <kernel/syscall.h>
#include <kernel/sched.h>
#include <kernel/errno.h>
#include <kernel/mmu.h>

i32 syscall_fork(struct cpu_state *state)
{
	struct process *proc = sched_get_current_proc();
	struct process *child = proc_clone(proc, state);

	if (!child)
		return -ENOMEM;

	proc_set_parent(child, proc);
	proc_set_syscall_ret(child->context, 0);

	int res = sched_schedule(child);

	if (!res) {
		res = child->pid;

		mutex_lock(&proc->lock);
		lst_append(&proc->children, child);
		mutex_unlock(&proc->lock);
	}

	/* pages are now CoW */
	mmu_invalidate_user();

	return res;
}

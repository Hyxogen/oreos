#include <kernel/syscall.h>
#include <kernel/sched.h>
#include <kernel/errno.h>
#include <kernel/mmu.h>

i32 syscall_fork(struct cpu_state *state)
{
	struct process *proc = sched_get_current_proc();
	struct process *child = proc_clone(proc, state);

	if (!child) {
		proc_release(proc);
		return -ENOMEM;
	}

	proc_set_parent(child, proc);
	proc_set_syscall_ret(child->context, 0);

	int res = sched_schedule(child);

	if (!res) {
		res = child->pid;

		spinlock_lock(&proc->lock);
		lst_append(&proc->children, child);
		spinlock_unlock(&proc->lock);
	} else {
		proc_release(child);
	}

	proc_release(proc);

	/* pages are now CoW */
	mmu_invalidate_user();

	return res;
}

#include <kernel/syscall.h>
#include <kernel/sched.h>
#include <kernel/errno.h>

static void lst_proc_del(void *proc)
{
	proc_release(proc);
}

static bool find_exited_proc(const void *proc_ptr, void *pid_ptr)
{
	int pid = *(int*)pid_ptr;
	const struct process *proc = proc_ptr;
	if (pid != -1 && pid != proc->pid)
		return false;

	return atomic_load_explicit(&proc->status, memory_order_relaxed) == DEAD;
}

i32 syscall_waitpid(struct cpu_state *state, int pid, int *wstatus, int options)
{
	(void)state;
	if (options)
		return -ENOTSUP;

	if (pid < -1 || pid == 0)
		return -ENOTSUP;

	struct process *proc = sched_get_current_proc();
	i32 res = -ECHILD;

	spinlock_lock(&proc->lock);

	while (!sched_has_pending_signals() && !lst_isempty(&proc->children)) {
		struct list_node *node = lst_find(&proc->children, find_exited_proc, &pid);

		if (node) {
			res = 0;
			struct process *child = node->_private;

			/* TODO properly set wstatus */
			if (wstatus && copy_to_user(wstatus, &child->exit_code, sizeof(child->exit_code)))
				res = -EINVAL;
			else
				lst_del(&proc->children, node, lst_proc_del);

			break;
		}

		condvar_wait(&proc->child_exited_cond, &proc->lock);
	}

	spinlock_unlock(&proc->lock);

	proc_release(proc);

	if (sched_has_pending_signals())
		res = -EINTR;

	return res;
}

#include <kernel/sync.h>
#include <kernel/libc/assert.h>
#include <kernel/sched.h>
#include <kernel/errno.h>

int condvar_init(struct condvar *cond)
{
	lst_init(&cond->waitlist);
	return 0;
}

int condvar_wait(struct condvar *cond, struct spinlock *mutex)
{
	assert(mutex);

	struct process *proc = sched_get_current_proc();

	sched_disable_preemption();
	struct list_node *node = lst_append(&cond->waitlist, proc);
	sched_enable_preemption();

	if (!node) {
		assert(0);
		return -ENOMEM;
	}

	spinlock_unlock(mutex);

	sched_goto_sleep();

	proc_release(proc); /* entry in waitlist was deleted, but we still have the reference */

	spinlock_lock(mutex);

	return 0;
}

int condvar_signal(struct condvar *cond)
{
	sched_disable_preemption();

	struct list_node *head = cond->waitlist.head;
	struct process *proc = NULL;

	if (head) {
		proc = head->_private;
		lst_del(&cond->waitlist, head, NULL);
	}

	sched_enable_preemption();

	if (proc)
		sched_wakeup(proc);

	return 0;
}

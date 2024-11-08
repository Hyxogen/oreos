#include <kernel/sync.h>
#include <kernel/libc/assert.h>
#include <kernel/sched.h>
#include <kernel/errno.h>
#include <kernel/kernel.h>

void condvar_init(struct condvar *cond)
{
	lst_init(&cond->waitlist);
}

void condvar_free(struct condvar *cond)
{
	sched_disable_preemption();
	lst_free(&cond->waitlist, NULL);
	sched_enable_preemption();
}

void condvar_wait(struct condvar *cond, struct mutex *mutex)
{
	assert(mutex);

	struct process *proc = sched_get_current_proc();
	sched_disable_preemption();
	struct list_node *node = lst_append(&cond->waitlist, proc);
	sched_enable_preemption();

	if (!node)
		panic("failed to setup condvar");

	sched_prepare_goto_sleep();

	mutex_unlock(mutex);

	/* we might have gottend signalled, but then we will just yield one
	 * quantum */
	sched_yield_here();

	mutex_lock(mutex);
}

void condvar_signal(struct condvar *cond)
{
	sched_disable_preemption();

	struct list_node *head = cond->waitlist.head;
	struct process *proc = NULL;

	if (head) {
		proc = head->_private;
		lst_del(&cond->waitlist, head, NULL);
	}

	if (proc)
		sched_wakeup(proc);

	sched_enable_preemption();
}

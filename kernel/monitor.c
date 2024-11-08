#include <kernel/sync.h>
#include <kernel/libc/assert.h>
#include <kernel/sched.h>
#include <kernel/kernel.h>

void monitor_init(struct monitor *monitor)
{
	lst_init(&monitor->waitlist);
}

static void monitor_free_proc(void *proc)
{
	proc_release(proc);
}

void monitor_free(struct monitor *monitor)
{
	sched_disable_preemption();
	lst_free(&monitor->waitlist, monitor_free_proc);
	sched_enable_preemption();
}

void monitor_wait(struct monitor *monitor, struct spinlock *lock)
{
	assert(lock);

	struct process *proc = sched_get_current_proc();

	sched_disable_preemption();
	struct list_node *node = lst_append(&monitor->waitlist, proc);
	sched_enable_preemption();

	if (!node)
		panic("failed to wait on monitor");

	sched_prepare_goto_sleep();

	spinlock_unlock(lock);

	/* we might have gottend signalled, but then we will just yield one
	 * quantum */
	sched_yield_here();

	spinlock_lock(lock);

	proc_release(proc);
}

void monitor_signal(struct monitor *monitor)
{
	sched_disable_preemption();

	struct list_node *head = monitor->waitlist.head;
	struct process *proc = NULL;

	if (head) {
		proc = head->_private;
		lst_del(&monitor->waitlist, head, NULL);
	}

	if (proc)
		sched_wakeup(proc);

	sched_enable_preemption();
}

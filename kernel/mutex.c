#include <kernel/sync.h>
#include <kernel/kernel.h>
#include <kernel/sched.h>
#include <kernel/errno.h>
#include <kernel/libc/assert.h>

int mutex_init(struct mutex *mtx, u32 flags)
{
	if (flags)
		oops("no mutex flags supported atm");

	atomic_flag_clear(&mtx->_locked);
	lst_init(&mtx->waitlist);
	return 0;
}

int mutex_lock(struct mutex *mtx)
{
	while (atomic_flag_test_and_set_explicit(&mtx->_locked, memory_order_acquire)) {
		struct process *proc = sched_get_current_proc();
		assert(proc);

		/* perhaps just use a spinlock? */
		sched_disable_preemption();
		struct list_node *node = lst_append(&mtx->waitlist, proc);
		sched_enable_preemption();

		if (!node) {
			assert(0);
			return -ENOMEM;
		}

		sched_goto_sleep();

		/* proc must be released here, not in unlock, as we might have
		 * been interrupted by a signal */
		/* XXX is that really a problem? */
		proc_release(proc);
	}
	return 0;
}

int mutex_unlock(struct mutex *mtx)
{
	atomic_flag_clear_explicit(&mtx->_locked, memory_order_release);

	sched_disable_preemption();

	struct list_node *head = mtx->waitlist.head;
	struct process *proc = NULL;

	if (head) {
		proc = head->_private;
		lst_del(&mtx->waitlist, head, NULL);
	}

	sched_enable_preemption();

	if (proc)
		sched_wakeup(proc);

	return 0;
}

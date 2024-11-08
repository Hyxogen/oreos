#include <kernel/sync.h>
#include <kernel/kernel.h>
#include <kernel/sched.h>
#include <kernel/errno.h>
#include <kernel/libc/assert.h>

void mutex_init(struct mutex *mtx, u32 flags)
{
	if (flags)
		oops("no mutex flags supported atm");

	spinlock_init(&mtx->lock);
	monitor_init(&mtx->monitor);
}

void mutex_free(struct mutex *mtx)
{
	sched_disable_preemption();

	assert(!mtx->locked);

	monitor_free(&mtx->monitor);

	sched_enable_preemption();
}

void mutex_lock(struct mutex *mtx)
{
	spinlock_lock(&mtx->lock);

	while (mtx->locked) {
		monitor_wait(&mtx->monitor, &mtx->lock);
	}

	mtx->locked = true;
	spinlock_unlock(&mtx->lock);
}

void mutex_unlock(struct mutex *mtx)
{
	spinlock_lock(&mtx->lock);

	assert(mtx->locked);
	mtx->locked = false;

	spinlock_unlock(&mtx->lock);
	monitor_signal(&mtx->monitor);
}

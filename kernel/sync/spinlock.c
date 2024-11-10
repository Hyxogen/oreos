#include <stdbool.h>
#include <kernel/sync.h>
#include <kernel/platform.h>

void spinlock_init(struct spinlock *lock)
{
	atomic_flag_clear(&lock->_locked);
}

/* TODO probably a good idea to disable preemption while holding a spinlock */
void spinlock_lock(struct spinlock *lock)
{
	while (atomic_flag_test_and_set_explicit(&lock->_locked, memory_order_acquire)) {
		halt();
	}
}

bool spinlock_trylock(struct spinlock *lock)
{
	return !atomic_flag_test_and_set_explicit(&lock->_locked, memory_order_acquire);
}

void spinlock_unlock(struct spinlock *lock)
{
	atomic_flag_clear_explicit(&lock->_locked, memory_order_release);
}

#include <stdbool.h>
#include <kernel/sync.h>
#include <kernel/platform.h>

int spinlock_init(struct spinlock *lock)
{
	atomic_flag_clear(&lock->_locked);
	return 0;
}

int spinlock_lock(struct spinlock *lock)
{
	while (atomic_flag_test_and_set_explicit(&lock->_locked, memory_order_acquire)) {
		halt();
	}
	return 0;
}

int spinlock_unlock(struct spinlock *lock)
{
	atomic_flag_clear_explicit(&lock->_locked, memory_order_release);
	return 0;
}

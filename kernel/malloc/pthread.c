#include <kernel/malloc/ma/internal.h>
#include <kernel/platform.h>

#if MA_USE_PTHREAD
#include <pthread.h>

int ma_init_mutex(ma_mtx *mtx) { return pthread_mutex_init(mtx, NULL); }

int ma_lock_mutex(ma_mtx *mtx) { return pthread_mutex_lock(mtx); }

int ma_unlock_mutex(ma_mtx *mtx) { return pthread_mutex_unlock(mtx); }
#else

int ma_init_mutex(ma_mtx *mtx)
{
	return spinlock_init(mtx);
}

int ma_lock_mutex(ma_mtx *mtx)
{
	return spinlock_lock(mtx);
}

int ma_unlock_mutex(ma_mtx *mtx)
{
	return spinlock_unlock(mtx);
}

#endif

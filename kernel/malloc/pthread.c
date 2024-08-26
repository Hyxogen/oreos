#include <kernel/malloc/ma/internal.h>


#if MA_USE_PTHREAD
#include <pthread.h>

int ma_init_mutex(ma_mtx *mtx) { return pthread_mutex_init(mtx, NULL); }

int ma_lock_mutex(ma_mtx *mtx) { return pthread_mutex_lock(mtx); }

int ma_unlock_mutex(ma_mtx *mtx) { return pthread_mutex_unlock(mtx); }
#else

int ma_init_mutex(ma_mtx *mtx)
{
	*mtx = 0;
	return 0;
}

int ma_lock_mutex(ma_mtx *mtx)
{
	ma_assert(!*mtx && "already locked");
	*mtx = 1;
	return 0;
}

int ma_unlock_mutex(ma_mtx *mtx)
{
	ma_assert(*mtx && "not locked");
	*mtx = 0;
	return 0;
}

#endif

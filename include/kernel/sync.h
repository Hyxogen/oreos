#ifndef __KERNEL_SYNC_H
#define __KERNEL_SYNC_H

#include <stdatomic.h>
#include <kernel/list.h>
#include <kernel/types.h>

struct spinlock {
	atomic_flag _locked;
};

void spinlock_init(struct spinlock *lock);
void spinlock_lock(struct spinlock *lock);
/* returns true if lock acquired */
bool spinlock_trylock(struct spinlock *lock);
void spinlock_unlock(struct spinlock *lock);

/* TODO give better name, its just a condvar but then with a spinlock */
struct monitor {
	struct list waitlist;
};

void monitor_init(struct monitor *monitor);
void monitor_wait(struct monitor *monitor, struct spinlock *lock);
void monitor_signal(struct monitor *monitor);
void monitor_free(struct monitor *monitor);

struct mutex {
	bool locked;
	struct spinlock lock;
	struct monitor monitor;
};

void mutex_init(struct mutex *mtx, u32 flags);
void mutex_free(struct mutex *mtx);
void mutex_lock(struct mutex *mtx);
bool mutex_trylock(struct mutex *mtx);
void mutex_unlock(struct mutex *mtx);

struct condvar {
	struct list waitlist;
};

void condvar_init(struct condvar *cond);
/* XXX MAKE NO OTHER REFERENCES EXIST TO THE CONDVAR, IT WILL NOT PROTECT YOU */
void condvar_free(struct condvar *cond);
void condvar_signal(struct condvar *cond);
/* TODO "normal" mutexes instead of spinlocks */
void condvar_wait(struct condvar *cond, struct mutex *mutex);

#endif

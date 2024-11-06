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

struct condvar {
	struct list waitlist;
};

int condvar_init(struct condvar *cond);
/* XXX MAKE NO OTHER REFERENCES EXIST TO THE CONDVAR, IT WILL NOT PROTECT YOU */
void condvar_free(struct condvar *cond);
int condvar_signal(struct condvar *cond);
/* TODO "normal" mutexes instead of spinlocks */
int condvar_wait(struct condvar *cond, struct spinlock *mutex);

#endif

#ifndef __KERNEL_SPINLOCK_H
#define __KERNEL_SPINLOCK_H

#include <stdatomic.h>

struct spinlock {
	atomic_flag _locked;
};

int init_spinlock(struct spinlock *lock);
int spinlock_lock(struct spinlock *lock);
int spinlock_unlock(struct spinlock *lock);

#endif
#ifndef __KERNEL_SCHED_H
#define __KERNEL_SCHED_H

#include <stdatomic.h>
#include <stdbool.h>
#include <stddef.h>
#include <kernel/types.h>
#include <kernel/mmu.h>

#define PROC_FLAG_RING0 0x01
#define PROC_FLAG_RING3 0x02

#define PROC_FLAG_RING_MASK 0x03

#define SIGSEGV 11

enum proc_status {
	READY,
	RUNNING,
	DEAD,
};

struct process {
	int pid;

	int exit_code;
	enum proc_status status;

	void* kernel_stack;

	struct cpu_state *context;
	struct process *next;

	struct mm mm;

	u32 pending_signals;
	void (*sighandlers[32])(int);

	atomic_uint refcount;
};

//TODO remove out of scheduler
struct process *proc_create(void *start, u32 flags);
struct process *proc_copy(const struct process *proc);
void proc_free(struct process *proc);
void proc_prepare_switch(struct process *proc);

void init_sched(void);
__attribute__((noreturn)) void sched_start(void);

__attribute__((noreturn)) void sched_yield(struct cpu_state *state);

int sched_kill(struct process *proc, int exit_code);
int sched_schedule(struct process *proc);

struct process *sched_get_current_proc(void);
void proc_release(struct process *proc);
void proc_get(struct process *proc);

#endif

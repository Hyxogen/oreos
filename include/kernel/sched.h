#ifndef __KERNEL_SCHED_H
#define __KERNEL_SCHED_H

#include <stdbool.h>
#include <kernel/types.h>

#define PROC_FLAG_RING0 0x01
#define PROC_FLAG_RING3 0x02

#define PROC_FLAG_RING_MASK 0x03

enum proc_status {
	READY,
	RUNNING,
	DEAD,
};

struct process {
	int pid;
	int exit_code;
	void* kernel_stack;
	enum proc_status status;
	struct cpu_state *context;
	struct process *next;
};
//TODO remove out of scheduler
struct process *proc_create(void *start, u32 flags);
void proc_free(struct process *proc);
void proc_prepare_switch(struct process *proc);

void init_sched(void);
int sched_proc(struct process *proc);
struct process *sched_schedule(struct cpu_state *state);
struct process *sched_cur(void);

__attribute__((noreturn))
void sched_start(void); /* TODO remove, should use sched_preempt */
__attribute__((noreturn))
void sched_preempt(struct cpu_state *state);

bool sched_set_preemption(bool enable);

#endif

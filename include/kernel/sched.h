#ifndef __KERNEL_SCHED_H
#define __KERNEL_SCHED_H

#include <stdbool.h>

enum proc_status {
	READY,
	RUNNING,
	DEAD,
};

struct process {
	int pid;
	enum proc_status status;
	struct cpu_state *context;
	struct process *next;
};
//TODO remove out of scheduler
struct cpu_state *proc_create(void *start);

bool sched_should_preempt(void);

void init_sched(void);
int sched_proc(struct cpu_state *ctx);
struct cpu_state *sched_schedule(struct cpu_state *state);
void sched_start(void);

#endif

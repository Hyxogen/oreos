#ifndef __KERNEL_SCHED_H
#define __KERNEL_SCHED_H

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


struct cpu_state *proc_create(void *start);
void sched_init(void);
int sched_proc(struct cpu_state *ctx);
struct cpu_state *do_schedule(struct cpu_state *state);
void sched_start(void);

#endif

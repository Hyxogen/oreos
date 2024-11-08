#ifndef __KERNEL_SCHED_H
#define __KERNEL_SCHED_H

#include <stdatomic.h>
#include <stdbool.h>
#include <stddef.h>
#include <kernel/types.h>
#include <kernel/mmu.h>
#include <kernel/list.h>

#define PROC_FLAG_RING0 0x01
#define PROC_FLAG_RING3 0x02

#define PROC_FLAG_RING_MASK 0x03

enum proc_status {
	READY,
	RUNNING,
	SLEEPING,
	DEAD,
};

struct process {
	int pid;
	atomic_int parent_pid;

	int exit_code;
	/* TODO why is status atomic? */
	_Atomic enum proc_status status;

	void *kernel_stack;

	struct cpu_state *context;
	struct process *next;

	struct mm mm;

	/* TODO blocked signals */
	u32 pending_signals;
	void (*signal_handlers[32])(int);

	struct list children;
	struct mutex lock;

	struct condvar child_exited_cond;

	unsigned int alarm;
};

//TODO remove out of scheduler
struct process *proc_create(void *start, u32 flags);
struct process *proc_clone(struct process *proc, const struct cpu_state *state);
void proc_free(struct process *proc);
void proc_prepare_switch(struct process *proc);
void proc_set_syscall_ret(struct cpu_state *state, size_t val);
void proc_set_parent(struct process *child, struct process *parent);
int proc_do_signal(struct process *proc, struct cpu_state *state);
int proc_do_sigreturn(struct process *proc, struct cpu_state *state);

bool sched_has_pending_signals();

void init_sched(void);
__attribute__((noreturn)) void sched_start(void);

__attribute__((noreturn)) void sched_yield(struct cpu_state *state);

__attribute__((noreturn))
void sched_do_kill(int exit_code);

int sched_signal(int pid, int signum);
int sched_schedule(struct process *proc);
__attribute__ ((noreturn))
void sched_resume(struct cpu_state *state);

void sched_yield_here(void);
/* TODO give better name, just sets the status of the thread to sleep */
void sched_prepare_goto_sleep(void);
void sched_goto_sleep(void);
void sched_wakeup(struct process *proc);
unsigned sched_set_alarm(struct process *proc, unsigned timepoint);

int sched_getpid(void);

struct process *sched_get_current_proc(void);
struct process *sched_get_init(void);

/* returns if preemption was enabled */
bool sched_disable_preemption(void);
/* returns if preemption is now enabled */
bool sched_enable_preemption(void);

/* TODO make a proper clock */
unsigned sched_get_time(void);

#endif

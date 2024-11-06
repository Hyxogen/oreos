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

enum proc_status {
	READY,
	RUNNING,
	DEAD,
};

struct process {
	int pid;

	int exit_code;
	enum proc_status status;

	void *kernel_stack;

	struct cpu_state *context;
	struct process *next;

	struct mm mm;

	atomic_uint refcount;

	/* TODO blocked signals */
	u32 pending_signals;
	void (*signal_handlers[32])(int);
};

//TODO remove out of scheduler
struct process *proc_create(void *start, u32 flags);
struct process *proc_clone(struct process *proc, const struct cpu_state *state);
void proc_free(struct process *proc);
void proc_prepare_switch(struct process *proc);
void proc_release(struct process *proc);
void proc_get(struct process *proc);
void proc_set_syscall_ret(struct cpu_state *state, size_t val);
int proc_do_signal(struct process *proc, struct cpu_state *state);
int proc_do_sigreturn(struct process *proc, struct cpu_state *state);

void init_sched(void);
__attribute__((noreturn)) void sched_start(void);

__attribute__((noreturn)) void sched_yield(struct cpu_state *state);

__attribute__ ((deprecated("use sched_signal")))
int sched_kill(struct process *proc, int exit_code);

void sched_signal(struct process *proc, int signum);
int sched_schedule(struct process *proc);
__attribute__ ((noreturn))
void sched_resume(struct cpu_state *state);

void sched_goto_sleep(void);
void sched_wakeup(struct process *proc);

int sched_getpid(void);

struct process *sched_get_current_proc(void);
struct process *sched_get(int pid);

/* returns if preemption was enabled */
bool sched_disable_preemption(void);
/* returns if preemption is now enabled */
bool sched_enable_preemption(void);

#endif

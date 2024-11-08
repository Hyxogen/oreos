#include <kernel/syscall.h>
#include <kernel/sched.h>
#include <kernel/errno.h>

i32 syscall_alarm(struct cpu_state *state, unsigned int seconds)
{
	(void) state;

	struct process *proc = sched_get_current_proc();
	unsigned now = sched_get_time();

	/* TODO don't hardcode granularity */
	unsigned prev = sched_set_alarm(proc, now + (seconds * 1000));

	return prev;
}

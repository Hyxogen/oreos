#include <kernel/syscall.h>
#include <kernel/sched.h>
#include <kernel/libc/assert.h>

i32 syscall_exit(int exit_code)
{
	/* we're gonna access the sched stuff, make sure we don't cause races */
	sched_set_preemption(false);

	struct process *cur = sched_cur();
	assert(cur);

	cur->exit_code = exit_code;
	cur->status = DEAD;
	sched_preempt(cur->context);
}

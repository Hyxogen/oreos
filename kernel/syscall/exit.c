#include <kernel/syscall.h>
#include <kernel/sched.h>
#include <kernel/libc/assert.h>

i32 syscall_exit(int exit_code)
{
	struct process *cur = sched_get_current_proc();
	assert(cur);

	sched_kill(cur, exit_code);
	sched_yield(NULL);
}

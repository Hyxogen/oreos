#include <kernel/syscall.h>
#include <kernel/sched.h>
#include <kernel/errno.h>
#include <kernel/libc/assert.h>

i32 syscall_pause(void)
{
	sched_goto_sleep();
	assert(sched_has_pending_signals());
	return -EINTR;
}

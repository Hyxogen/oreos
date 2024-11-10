#include <kernel/syscall.h>
#include <kernel/sched.h>

i32 syscall_getuid(void)
{
	struct process *proc = sched_get_current_proc();
	return proc->uid;
}

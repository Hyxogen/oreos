#include <kernel/syscall.h>
#include <kernel/sched.h>

i32 syscall_sched_yield(void)
{
	sched_yield_here();
	return 0;
}

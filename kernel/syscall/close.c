#include <kernel/syscall.h>
#include <kernel/sched.h>
#include <kernel/errno.h>

i32 syscall_close(struct cpu_state *state, int fd)
{
	(void) state;

	if (fd < 0 || fd >= PROC_MAX_OPEN_FILES)
		return -EBADF;

	if (fd == 0)
		return -EBADF;

	struct process *proc = sched_get_current_proc();
	struct file *f = proc->files[fd];

	if (!f)
		return -EBADF;
	return vfs_close(f);
}

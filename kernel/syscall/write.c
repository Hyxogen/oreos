#include <stddef.h>
#include <kernel/syscall.h>
#include <kernel/errno.h>
#include <kernel/tty.h>
#include <kernel/platform.h>
#include <kernel/sched.h>

i32 syscall_write(struct cpu_state *state, int fd, const void *buf, size_t nbytes)
{
	(void)state;
	(void)fd;
	(void)nbytes;

	if (fd < 0 || fd >= PROC_MAX_OPEN_FILES)
		return -EINVAL;

	
	if (fd == 0) {
		const char *cbuf = buf;

		char tmp[64];
		i32 nwritten = 0;
		size_t size = 0;

		while (nbytes > 0) {
			size = nbytes > sizeof(tmp) ? sizeof(tmp) : nbytes;

			if (copy_from_user(tmp, cbuf, size))
				return -EINVAL;

			term_write(term_get_primary(), tmp, size);

			nbytes -= size;
			nwritten += size;
			cbuf += size;
		}
		return nwritten;
	}

	struct process *proc = sched_get_current_proc();
	struct file *f = proc->files[fd];
	if (!f)
		return -EBADF;
	return vfs_write(f, buf, nbytes);
}

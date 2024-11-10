#include <kernel/syscall.h>
#include <kernel/fs/socket.h>
#include <kernel/sched.h>
#include <kernel/errno.h>
#include <kernel/malloc/malloc.h>
#include <kernel/libc/assert.h>

i32 syscall_socketpair(struct cpu_state *state, int domain, int type, int protocol, int sv[2])
{
	(void) state;

	if (domain != AF_UNIX && type != SOCK_STREAM && protocol != 0)
		return -ENOTSUP;

	struct process *proc = sched_get_current_proc();

	int fds[2] = {-1, -1};

	fds[0] = proc_alloc_fd(proc);
	fds[1] = proc_alloc_fd(proc);

	if (fds[0] < 0 || fds[1] < 0) {
		proc_free_fd(proc, fds[0]);
		proc_free_fd(proc, fds[1]);
		return -ENOMEM; /* TODO properly set errno */
	}

	struct socket *sockets[2];
	int res = socket_createpair(sockets);
	assert(!res);
	if (res)
		return -1;

	sched_disable_preemption(); /* TODO use proc spinlock */

	/* TODO abstraction leak, make socket init the file on its own */
	struct file *files[2] = { proc->files[fds[0]], proc->files[fds[1]] };

	files[0]->fops = files[1]->fops = &socket_ops;
	files[0]->priv = sockets[0];
	files[1]->priv = sockets[1];

	atomic_init(&files[0]->refcount, 1);
	atomic_init(&files[1]->refcount, 1);

	sched_enable_preemption();

	return copy_to_user(sv, fds, sizeof(fds));
 }

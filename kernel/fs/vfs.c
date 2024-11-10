#include <kernel/fs/vfs.h>
#include <kernel/malloc/malloc.h>
#include <kernel/sched.h>
#include <kernel/errno.h>
#include <kernel/platform.h>
#include <kernel/libc/assert.h>

void file_free(struct file *f)
{
	if (atomic_fetch_sub_explicit(&f->refcount, 1, memory_order_relaxed) == 1) {
		assert(f->fops);
		assert(f->fops->destroy);
		f->fops->destroy(f);
		kfree(f);
	}
}

ssize_t vfs_read(struct file *f, void *buf, size_t nbytes)
{
	struct process *proc = sched_get_current_proc();

	if (nbytes > PROC_IOBUF_SIZE)
		nbytes = PROC_IOBUF_SIZE;

	if (!f->fops->read)
		return -ENOTSUP;
	
	ssize_t nread = f->fops->read(f, proc->iobuf, nbytes);
	if (nread <= 0)
		return nread;

	int res = copy_to_user(buf, proc->iobuf, nbytes);
	if (res)
		return res;
	return nread;
}

ssize_t vfs_write(struct file *f, const void *buf, size_t nbytes)
{
	struct process *proc = sched_get_current_proc();

	if (nbytes > PROC_IOBUF_SIZE)
		nbytes = PROC_IOBUF_SIZE;

	if (!f->fops->write)
		return -ENOTSUP;

	int res = copy_from_user(proc->iobuf, buf, nbytes);
	if (res)
		return res;

	return f->fops->write(f, proc->iobuf, nbytes);
}

int vfs_close(struct file *f)
{
	assert(f);
	file_free(f);
	return 0;
}

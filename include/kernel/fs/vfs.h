#ifndef __KERNEL_FS_VFS_H
#define __KERNEL_FS_VFS_H

#include <stdatomic.h>
#include <kernel/types.h>
#include <kernel/sync.h>

/* must be a power of two */
#define VFS_PIPE_SIZE 256

struct file;

struct file_ops {
	ssize_t (*read)(struct file *f, void *buf, size_t nbytes);
	ssize_t (*write)(struct file *f, const void *buf, size_t nbytes);
	void (*destroy)(struct file *f); /* TODO remove */
};

struct file {
	const struct file_ops *fops;
	void *priv;
	atomic_uint refcount;
};

struct pipe {
	unsigned char buf[VFS_PIPE_SIZE];
	size_t read_head;
	size_t write_head;

	struct condvar read_not_empty;
	struct condvar write_not_full;
	struct mutex mtx;
	atomic_uint refcount;
};

void file_free(struct file *f);

int pipe_create(struct pipe **dest);

ssize_t vfs_read(struct file *f, void *buf, size_t nbytes);
ssize_t vfs_write(struct file *f, const void *buf, size_t nbytes);
int vfs_close(struct file *f);

extern const struct file_ops pipe_ops;
#endif

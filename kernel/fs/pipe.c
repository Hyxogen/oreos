#include <kernel/fs/vfs.h>
#include <kernel/fs/socket.h>
#include <kernel/sched.h>
#include <kernel/errno.h>
#include <kernel/libc/assert.h>
#include <kernel/libc/string.h>
#include <kernel/malloc/malloc.h>

/* TODO send sigpipe when needed */

static size_t _pipe_nreadable(const struct pipe *pipe)
{
	if (pipe->read_head < pipe->write_head)
		return pipe->write_head - pipe->read_head;
	else if (pipe->read_head > pipe->write_head)
		return pipe->write_head + VFS_PIPE_SIZE - pipe->read_head;
	return 0;
}

static size_t _pipe_nwritable(const struct pipe *pipe)
{
	return VFS_PIPE_SIZE - _pipe_nreadable(pipe);
}

static size_t _pipe_advance_read(struct pipe *pipe, size_t nbytes)
{
	size_t size = _pipe_nreadable(pipe);
	assert(nbytes <= size);
	pipe->read_head = (pipe->read_head + nbytes) % VFS_PIPE_SIZE;
	return nbytes;
}

static size_t _pipe_advance_write(struct pipe *pipe, size_t nbytes)
{
	size_t size = _pipe_nwritable(pipe);
	assert(nbytes <= size);
	pipe->write_head = (pipe->write_head + nbytes) % VFS_PIPE_SIZE;
	return nbytes;
}

static ssize_t _pipe_read(struct pipe *pipe, void *buf, size_t nbytes)
{
	mutex_lock(&pipe->mtx);

	ssize_t nread = 0;
	while (!sched_has_pending_signals()) {
		size_t consumable = _pipe_nreadable(pipe);
		if (consumable > nbytes)
			consumable = nbytes;

		if (consumable) {
			if (pipe->write_head < pipe->read_head) {
				size_t n = VFS_PIPE_SIZE - pipe->read_head;
				if (n > consumable)
					n = consumable;

				buf = mempcpy(buf, &pipe->buf[pipe->read_head], n);
				nread += _pipe_advance_read(pipe, n);
				consumable -= n;
			}

			memcpy(buf, &pipe->buf[pipe->read_head], consumable);
			nread += _pipe_advance_read(pipe, consumable);
			break;
		} else {
			condvar_wait(&pipe->read_not_empty, &pipe->mtx);
		}
	}

	if (nread)
		condvar_signal(&pipe->write_not_full);

	mutex_unlock(&pipe->mtx);

	if (!nread)
		return -EINTR;

	return nread;
}

static ssize_t pipe_read(struct file *f, void *buf, size_t nbytes)
{
	return _pipe_read(f->priv, buf, nbytes);
}

static ssize_t _pipe_write(struct pipe *pipe, const void *buf, size_t nbytes)
{
	mutex_lock(&pipe->mtx);

	ssize_t nwritten = 0;
	while (!sched_has_pending_signals()) {
		size_t writable = _pipe_nwritable(pipe);
		/* TODO sigpipe */

		if (writable > nbytes)
			writable = nbytes;

		if (writable) {
			if (pipe->read_head < pipe->write_head) {
				size_t n = VFS_PIPE_SIZE - pipe->write_head;
				if (n > writable)
					n = writable;

				memcpy(&pipe->buf[pipe->write_head], buf, n);
				buf = (char*) buf + n;
				nwritten = _pipe_advance_write(pipe, n);
				writable -= n;
			}

			memcpy(&pipe->buf[pipe->write_head], buf, writable);
			nwritten = _pipe_advance_write(pipe, writable);
			break;
		} else {
			condvar_wait(&pipe->write_not_full, &pipe->mtx);
		}
	}

	if (nwritten)
		condvar_signal(&pipe->read_not_empty);

	mutex_unlock(&pipe->mtx);

	if (!nwritten)
		return -EINTR;

	return nwritten;
}

static ssize_t pipe_write(struct file *f, const void *buf, size_t nbytes)
{
	return _pipe_write(f->priv, buf, nbytes);
}

static void pipe_init(struct pipe *pipe)
{
	memset(pipe->buf, 0xae, VFS_PIPE_SIZE);
	pipe->read_head = 0;
	pipe->write_head = 0;
	atomic_init(&pipe->refcount, 0);

	condvar_init(&pipe->read_not_empty);
	condvar_init(&pipe->write_not_full);
	mutex_init(&pipe->mtx, 0);
}

static void _pipe_free(struct pipe *pipe)
{
	if (atomic_fetch_sub_explicit(&pipe->refcount, 1, memory_order_relaxed) == 1) {
		condvar_free(&pipe->read_not_empty);
		condvar_free(&pipe->write_not_full);
		mutex_free(&pipe->mtx);
	} else {
		condvar_signal(&pipe->read_not_empty);
		condvar_signal(&pipe->write_not_full);
	}
}

static void pipe_free(struct file *f)
{
	_pipe_free(f->priv);
}

int pipe_create(struct pipe **dest)
{
	assert(dest);
	*dest = kmalloc(sizeof(*(*dest)));

	if (*dest) {
		pipe_init(*dest);
		return 0;
	}

	return -ENOMEM;
}

static void socket_init(struct socket *socket)
{
	socket->read = NULL;
	socket->write = NULL;
}

int socket_createpair(struct socket *sockets[2])
{
	assert(sockets);

	struct socket *tmp = kcalloc(sizeof(struct socket), 2);
	if (!tmp)
		return -ENOMEM;
	sockets[0] = &tmp[0];
	sockets[1] = &tmp[1];

	socket_init(sockets[0]);
	socket_init(sockets[1]);

	int res = pipe_create(&sockets[0]->read);
	if (res)
		return res;

	res = pipe_create(&sockets[0]->write);
	if (res) {
		_pipe_free(sockets[0]->read);
		return res;
	}

	atomic_fetch_add(&sockets[0]->read->refcount, 1);
	atomic_fetch_add(&sockets[0]->write->refcount, 1);

	sockets[1]->read = sockets[0]->write;
	sockets[1]->write = sockets[0]->read;
	return 0;
}

const struct file_ops pipe_ops = {
	.read = pipe_read,
	.write = pipe_write,
	.destroy = pipe_free,
};

static void socket_free(struct file *f)
{
	struct socket *socket = f->priv;
	_pipe_free(socket->read);
	_pipe_free(socket->write);
}

ssize_t socket_read(struct file *f, void *buf, size_t nbytes)
{
	struct socket *socket = f->priv;
	return _pipe_read(socket->read, buf, nbytes);
}

ssize_t socket_write(struct file *f, const void *buf, size_t nbytes)
{
	struct socket *socket = f->priv;
	return _pipe_write(socket->write, buf, nbytes);
}

const struct file_ops socket_ops = {
	.read = socket_read,
	.write = socket_write,
	.destroy = socket_free,
};

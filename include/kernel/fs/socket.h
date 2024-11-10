#ifndef __KERNEL_FS_SOCKET_H
#define __KERNEL_FS_SOCKET_H

#include <kernel/fs/vfs.h>

#define AF_UNIX 1
#define SOCK_STREAM 1

struct socket {
	struct pipe *read;
	struct pipe *write;
};

int socket_createpair(struct socket *sockets[2]);

extern const struct file_ops socket_ops;
#endif

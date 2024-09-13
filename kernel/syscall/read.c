#include <stddef.h>
#include <kernel/syscall.h>
#include <kernel/errno.h>
#include <kernel/tty.h>

//TODO SANITIZE INPUT!!
i32 read(int fd, void *buf, size_t nbytes)
{
	//TODO make sure that the process owns the buffer
	return -ENOTSUP;
}

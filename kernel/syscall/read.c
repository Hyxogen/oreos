#include <stddef.h>
#include <kernel/syscall.h>
#include <kernel/errno.h>
#include <kernel/tty.h>
#include <kernel/ps2.h>
#include <kernel/sched.h>

//TODO SANITIZE INPUT!!
i32 syscall_read(int fd, void *buf, size_t nbytes)
{
	(void)fd;
	/* TODO make sure that the process owns the buffer */

	size_t n = ps2_read(buf, nbytes);
	return (i32) n;
}

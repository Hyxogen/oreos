#include <stddef.h>
#include <kernel/syscall.h>
#include <kernel/errno.h>
#include <kernel/tty.h>

//TODO SANITIZE INPUT!!
i32 syscall_write(int fd, const void *buf, size_t nbytes)
{
	//TODO make sure that the process owns the buffer
	term_write(term_get_primary(), buf, nbytes);
	return -ENOTSUP;
}

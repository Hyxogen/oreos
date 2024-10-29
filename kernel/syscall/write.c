#include <stddef.h>
#include <kernel/syscall.h>
#include <kernel/errno.h>
#include <kernel/tty.h>
#include <kernel/platform.h>

//TODO SANITIZE INPUT!!
i32 syscall_write(int fd, const void *buf, size_t nbytes)
{
	(void)fd;
	(void)nbytes;
	//TODO make sure that the process owns the buffer
	//TODO write more than 1 byte
	
	u8 byte;
	if (get_user1(&byte, buf))
		return -EINVAL;
	term_write(term_get_primary(), (void*) &byte, 1);
	return 1;
}

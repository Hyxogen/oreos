#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <kernel/mmu.h>
#include <kernel/sched.h>
#include <kernel/platform.h>

static bool check_user_buf(const void *user_ptr, size_t len)
{
	uintptr_t ptr = (uintptr_t)user_ptr;

	if (ptr + len < ptr)
		return false;
	return ptr < MMU_KERNEL_START && ptr + len <= MMU_KERNEL_START;
}

int __put_user1(void *dest, u8 val);
int __get_user1(void *dest, const void *src);
int __user_memcpy(void *dest, const void *src, size_t nbytes);

int put_user1(void *dest, u8 val)
{
	if (!check_user_buf(dest, 1)) {
		return -1;
	}
	return __put_user1(dest, val);
}

int get_user1(u8 *dest, const void *src)
{
	if (!check_user_buf(src, 1))
		return -1;
	return __get_user1(dest, src);
}

int copy_to_user(void *dest, const void *src, size_t nbytes)
{
	if (!check_user_buf(dest, nbytes))
		return -1;
	return __user_memcpy(dest, src, nbytes);
}

int copy_from_user(void *dest, const void *src, size_t nbytes)
{
	if (!check_user_buf(src, nbytes))
		return -1;
	return __user_memcpy(dest, src, nbytes);
}

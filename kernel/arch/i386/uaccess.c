#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <kernel/mmu.h>
#include <kernel/sched.h>
#include <kernel/platform.h>

__attribute__((section(".user.text")))
static bool check_user_buf(const void *user_ptr, size_t len)
{
	uintptr_t ptr = (uintptr_t)user_ptr;

	if (ptr + len < ptr)
		return false;
	return ptr < MMU_KERNEL_START && ptr + len < MMU_KERNEL_START;
}

__attribute__((noinline, section(".user.text")))
void* put_user1(void *dest, u8 val)
{
	if (!check_user_buf(dest, 1)) {
		/* TODO properly send signal */
		*(volatile int *)0 = 0;
	}
	*(u8*)dest = val;
	return (u8*) dest +1;
}

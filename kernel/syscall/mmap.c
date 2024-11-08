#include <kernel/syscall.h>
#include <kernel/errno.h>
#include <kernel/mmu.h>
#include <kernel/sched.h>

#define MAP_PROT_READ 0x1
#define MAP_PROT_WRITE 0x2

#define MAP_PRIVATE 0x01
#define MAP_ANONYMOUS 0x02

i32 syscall_mmap(struct cpu_state *state, uintptr_t addr, size_t len, int prot, int flags, int fd, i32 off)
{
	(void) state;
	(void) flags;
	(void) fd;
	(void) off;

	if (!len || addr + len < addr || addr >= MMU_KERNEL_START ||
	    addr + len >= MMU_KERNEL_START)
		return -EINVAL;
	if (!prot)
		return -EINVAL;
	if (!(flags & MAP_PRIVATE) || !(flags & MAP_ANONYMOUS))
		return -ENOTSUP;

	struct process *proc = sched_get_current_proc();

	u32 vma_flags = 0;
	if (prot & MAP_PROT_READ)
		vma_flags |= VMA_MAP_PROT_READ;
	if (prot & MAP_PROT_WRITE)
		vma_flags |= VMA_MAP_PROT_WRITE;

	i32 res = vma_map(&proc->mm, &addr, len, vma_flags);

	if (res)
		return res;
	return (i32) addr;
}

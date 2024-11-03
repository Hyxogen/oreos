#include <kernel/elf.h>
#include <kernel/libc/assert.h>
#include <kernel/libc/string.h>
#include <kernel/mmu.h>
#include <kernel/sched.h>
#include <kernel/types.h>

struct process *load_elf(const void *data, size_t nbytes)
{
	(void)nbytes;
	const Elf32_Ehdr *ehdr = data;

	struct process *proc =
	    proc_create((void *)(uintptr_t)ehdr->e_entry, PROC_FLAG_RING3);
	if (!proc)
		return NULL;

	/* TODO SANITIZE */
	assert(ehdr->e_type == ET_EXEC);
	assert(ehdr->e_machine == EM_386);

	for (unsigned i = 0; i < ehdr->e_phnum; i++) {
		const Elf32_Phdr *phdr = (void *)((u8 *)data + ehdr->e_phoff +
						  i * ehdr->e_phentsize);

		if (phdr->p_type != PT_LOAD)
			continue;

		/* TODO properly set permissions */
		u32 flags = VMA_MAP_FIXED_NOREPLACE | VMA_MAP_PROT_WRITE |
			    VMA_MAP_PROT_READ;

		uintptr_t vaddr = phdr->p_vaddr;

		int res = vma_map(&proc->mm, &vaddr, phdr->p_memsz, flags);
		assert(!res);
		res = vma_map_now(&proc->mm, vaddr, phdr->p_memsz);

		memset((void *)vaddr, 0, phdr->p_memsz);
		memcpy((void *)vaddr, (u8 *)data + phdr->p_offset,
		       phdr->p_filesz);
	}

	return proc;
}

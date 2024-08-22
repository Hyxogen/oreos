#ifndef __KERNEL_ARCH_I386_MM_H
#define __KERNEL_ARCH_I386_MM_H

#include <stdbool.h>
#include <stddef.h>

#include <kernel/types.h>
#include <kernel/mm.h>

// the PHYSICAL address the kernel code is loaded
extern u8 _kernel_pstart;
// the VIRTUAL address the kernel code is loaded
extern u8 _kernel_vstart;
// the VIRTUAL address the kernel code ends
extern u8 _kernel_vend;
// TODO rename to something like "high_mem"
// the start of the VIRTUAL address for kernel stuff
extern u8 _kernel_addr;

struct mm_pte {
	bool present : 1;
	bool rw : 1;
	bool user : 1;
	u32 pwt : 1;
	u32 pcd : 1;
	bool accessed : 1;
	bool dirty : 1;
	u32 pat : 1;
	bool global : 1;
	u32 avl : 3;
	u32 pfn : 20;
};

struct mm_pde {
	bool present : 1;
	bool rw : 1;
	bool user : 1;
	bool pwt : 1;
	bool pcd : 1;
	bool accessed : 1;
	u32 avl2 : 1;
	u32 ps : 1;
	u32 avl1 : 4;
	u32 pfn : 20;
};

_Static_assert(sizeof(struct mm_pte) == sizeof(u32), "basic assumption");

#define MM_PTE_OFFSET 12
#define MM_PDE_OFFSET (MM_PTE_OFFSET + 10)
#define MM_MAX_PAGES (1024 * 1024)

#define MM_PTE_MASK (0x3ff << MM_PTE_OFFSET)
#define MM_PDE_MASK (0x3ff << MM_PDE_OFFSET)

#define MM_PTE_IDX(vaddr) (((uintptr_t)(vaddr) & MM_PTE_MASK) >> MM_PTE_OFFSET)
#define MM_PDE_IDX(vaddr) (((uintptr_t)(vaddr) & MM_PDE_MASK) >> MM_PDE_OFFSET)

#define MM_INVALID_PFN ((size_t) - 1)
#define MM_INVALID_PAGE ((void*) -1)

#define MM_PADDR_TO_PFN(paddr) ((uintptr_t) (paddr) >> (MM_PTE_OFFSET))

#endif

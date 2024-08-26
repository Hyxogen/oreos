#ifndef __KERNEL_ARCH_I386_MMU_H
#define __KERNEL_ARCH_I386_MMU_H

#include <stdbool.h>
#include <stddef.h>

#include <kernel/types.h>
#include <kernel/mmu.h>

// the PHYSICAL address the kernel code is loaded
extern u8 _kernel_pstart;
// the VIRTUAL address the kernel code is loaded
extern u8 _kernel_vstart;
// the VIRTUAL address the kernel code ends
extern u8 _kernel_vend;
// TODO rename to something like "high_mem"
// the start of the VIRTUAL address for kernel stuff
extern u8 _kernel_addr;

struct mmu_pte {
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

struct mmu_pde {
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

_Static_assert(sizeof(struct mmu_pte) == sizeof(u32), "basic assumption");

void mmu_flush_tlb(void);

#define MMU_PTE_SHIFT 12
#define MMU_PDE_SHIFT (MMU_PTE_SHIFT + 10)
#define MMU_MAX_PAGES (1024 * 1024)

#define MMU_PTE_MASK (0x3ff << MMU_PTE_SHIFT)
#define MMU_PDE_MASK (0x3ff << MMU_PDE_SHIFT)

#define MMU_PTE_IDX(vaddr) (((uintptr_t)(vaddr) & MMU_PTE_MASK) >> MMU_PTE_SHIFT)
#define MMU_PDE_IDX(vaddr) (((uintptr_t)(vaddr) & MMU_PDE_MASK) >> MMU_PDE_SHIFT)

#define MMU_INVALID_PFN ((size_t) - 1)
#define MMU_INVALID_PAGE ((void*) -1)

#define MMU_PADDR_TO_PFN(paddr) ((uintptr_t) (paddr) >> (MMU_PTE_SHIFT))
#define MMU_PFN_TO_PADDR(pfn)   ((uintptr_t) (pfn) << (MMU_PTE_SHIFT))

#endif

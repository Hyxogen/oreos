#ifndef __KERNEL_MMU_H
#define __KERNEL_MMU_H

#include <boot/multiboot2.h>
#include <kernel/types.h>
#include <stddef.h>

#define MMU_PAGESIZE 0x1000

#define MMU_MAP_RDONLY 0x1
#define MMU_MAP_NOALLOC 0x2
#define MMU_MAP_FIXED 0x4

#define MMU_ADDRSPACE_KERNEL 0x1
#define MMU_ADDRSPACE_USER 0x2

#define __MMU_FREE_BIT 0x80

#define MMU_ALLOC_FIXED 0x1

struct page {
	u8 flags;
};

struct page *mmu_alloc_pageframe(uintptr_t hint, size_t nframes, u32 flags);
void mmu_free_pageframe(struct page *page, size_t nframes);

struct page *mmu_paddr_to_page(uintptr_t paddr);
uintptr_t mmu_page_to_paddr(const struct page *page);

void *mmu_map_pages(void *vaddr, struct page *frame, size_t nframes, int addrspace,
	      u32 flags);
void *mmu_map(void *vaddr, uintptr_t paddr, size_t len, int addrspace, u32 flags);
void mmu_unmap(void *vaddr, size_t len);

void init_mmu(const struct mb2_info *info);

#endif

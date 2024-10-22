#ifndef __KERNEL_MMU_H
#define __KERNEL_MMU_H

#include <boot/multiboot2.h>
#include <kernel/types.h>
#include <stddef.h>

#define MMU_PAGESIZE 0x1000

#define MMU_MAP_RDONLY 0x1
#define MMU_MAP_NOALLOC 0x2
#define MMU_MAP_FIXED 0x4
#define MMU_MAP_DOWNWARD 0x8

#define MMU_ADDRSPACE_KERNEL 0x1
#define MMU_ADDRSPACE_USER 0x2

#define __MMU_FREE_BIT 0x80

#define MMU_ALLOC_FIXED 0x1

#define PAGE_OFFSET(addr) ((uintptr_t) (addr) & (MMU_PAGESIZE - 1))

#define MMU_MAP_FAILED ((void*) -1)

#define MMU_KERNEL_START 0xc0000000
#define MMU_USER_START 0x00400000

#define MMU_UNMAP_IGNORE_UNMAPPED 0x1

struct page {
	u8 flags;
};

struct mm_area {
	struct page *page;
	uintptr_t start;
	uintptr_t end;
	u32 flags;

	/* XXX probably a good idea to use some kind of self balancing tree */
	struct mm_area *parent;
	struct mm_area *left;
	struct mm_area *right;
};

struct mm {
	struct mm_area *root;
};

struct page *mmu_alloc_pageframe(uintptr_t hint, size_t nframes, u32 flags);
void mmu_free_pageframe(struct page *page, size_t nframes);

struct page *mmu_paddr_to_page(uintptr_t paddr);
uintptr_t mmu_page_to_paddr(const struct page *page);

void *mmu_map_pages(void *vaddr, struct page *frame, size_t nframes, int addrspace,
	      u32 flags);
void *mmu_map(void *vaddr, uintptr_t paddr, size_t len, int addrspace, u32 flags);
int mmu_unmap(void *vaddr, size_t len, u32 flags);

void init_mmu(void);

void *mmu_mmap(void *vaddr, size_t len, int addrspace, u32 flags);
void mmu_invalidate_user(void);

/* TODO this is a really bad name, probably best to merge with with the mmu */
void init_mm(void);

struct mm_area *mm_find_mapping(const struct mm_area *area, uintptr_t start,
				uintptr_t end);
int mm_map(struct mm *mm, uintptr_t *addr, size_t len, u32 flags);
int mm_unmap(struct mm *mm, uintptr_t addr, size_t len);
int mm_map_now(struct mm_area *area);

#endif

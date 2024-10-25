#ifndef __KERNEL_MMU_H
#define __KERNEL_MMU_H

#include <stdatomic.h>
#include <boot/multiboot2.h>
#include <kernel/types.h>
#include <stddef.h>
#include <stdbool.h>
#include <kernel/platform.h>
#include <kernel/irq.h>
#include <kernel/spinlock.h>

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
#define MMU_UNMAP_NO_DECR_REF 0x02

#define VMA_MAP_PROT_READ 0x01
#define VMA_MAP_PROT_WRITE 0x02
#define VMA_MAP_FIXED_NOREPLACE 0x04

/* just a generally good pageframe hint */
#define MMU_PAGEFRAME_HINT (16 * 1024 * 1024)

struct page {
	atomic_uint_least8_t refcount;
};

struct vma_area {
	struct page **pages;
	uintptr_t start;
	uintptr_t end;
	u32 flags;

	/* XXX probably a good idea to use some kind of self balancing tree */
	struct vma_area *parent;
	struct vma_area *left;
	struct vma_area *right;
};

struct mm {
	struct vma_area *root;
};

struct pagefault {
	uintptr_t addr;
	bool is_write;
	bool is_present;
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

int mmu_register_pagefault_handler(enum irq_result (*handler)(
    struct cpu_state *state, const struct pagefault *info));

void init_vma(void);

struct vma_area *vma_find_mapping(const struct vma_area *area, uintptr_t start,
				uintptr_t end);
int vma_map(struct mm *mm, uintptr_t *addr, size_t len, u32 flags);
int vma_unmap(struct mm *mm, uintptr_t addr, size_t len);
int vma_destroy(struct mm *mm);
/* TODO remove from public interface */
int vma_map_now_one(struct vma_area *area, uintptr_t addr);

#endif

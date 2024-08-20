// https://f.osdev.org/viewtopic.php?p=230374&sid=99b22aa6f322a817de79fb61778e78c6#p230374

#include "multiboot2.h"

#include <kernel/align.h>
#include <kernel/kernel.h>
#include <kernel/types.h>
// TODO rename to libk?
#include <lib/string.h>
#include <stdbool.h>
#include <stddef.h>

#define MM_PAGESIZE 0x1000
#define MM_PTE_OFFSET 12
#define MM_PDE_OFFSET (MM_PTE_OFFSET + 10)
#define MM_MAX_PAGES (1024 * 1024)

#define MM_PTE_MASK (0x3ff << MM_PTE_OFFSET)
#define MM_PDE_MASK (0x3ff << MM_PDE_OFFSET)

#define MM_PTE_IDX(vaddr) (((uintptr_t)(vaddr) & MM_PTE_MASK) >> MM_PTE_OFFSET)
#define MM_PDE_IDX(vaddr) (((uintptr_t)(vaddr) & MM_PDE_MASK) >> MM_PDE_OFFSET)

#define MM_MAP_RDONLY 0x1
#define MM_MAP_NOALLOC 0x2

#define MM_PADDR_TO_PFN(paddr) ((paddr) >> (MM_PTE_OFFSET))

#define MM_INVALID_PFN ((size_t) - 1)

struct mm_pte {
	bool present : 1;
	bool rw : 1;
	bool user : 1;
	uint32_t pwt : 1;
	uint32_t pcd : 1;
	bool accessed : 1;
	bool dirty : 1;
	uint32_t pat : 1;
	bool global : 1;
	uint32_t avl : 3;
	uint32_t pfn : 20;
};

struct mm_pde {
	bool present : 1;
	bool rw : 1;
	bool user : 1;
	bool pwt : 1;
	bool pcd : 1;
	bool accessed : 1;
	uint32_t avl2 : 1;
	uint32_t ps : 1;
	uint32_t avl1 : 4;
	uint32_t pfn : 20;
};

_Static_assert(sizeof(struct mm_pte) == sizeof(u32), "basic assumption");

extern u8 _kernel_addr;
extern u8 _kernel_start;
extern u8 _kernel_end;

// **********************************************************************
// Physical memory
// **********************************************************************

static u8 mm_page_bitmap[0x20000];

static size_t mm_get_pfn_idx(size_t pfn)
{
	return pfn >> 10;
}

static size_t mm_get_pfn_off(size_t pfn)
{
	return pfn & 0x3ff;
}

static void __mm_mark_pageframe(size_t pfn, size_t count, bool used)
{
	while (count--) {
		size_t idx = mm_get_pfn_idx(pfn);
		size_t off = mm_get_pfn_off(pfn);

		u8 mask = 1 << off;

		mm_page_bitmap[idx] &= ~mask;
		mm_page_bitmap[idx] |= !used * mask;

		pfn++;
	}
}

static bool mm_pageframe_used(size_t pfn)
{
	size_t idx = mm_get_pfn_idx(pfn);
	size_t off = mm_get_pfn_off(pfn);

	return mm_page_bitmap[idx] & (1 << off);
}

static void mm_free_pageframe(uintptr_t pfn, size_t count)
{
	__mm_mark_pageframe(pfn, count, false);
}

size_t mm_alloc_pageframe(size_t count)
{
	size_t pfn = 0;
	size_t len = 0;

	for (size_t cur = 0; cur < MM_MAX_PAGES; cur++) {
		if (mm_pageframe_used(cur)) {
			len = 0;
			continue;
		}

		if (!len)
			pfn = cur;

		len += 1;

		if (len == count) {
			__mm_mark_pageframe(pfn, count, true);
			return pfn;
		}
	}
	return MM_INVALID_PFN;
}

// **********************************************************************
// Virtual memory
// **********************************************************************

void mm_write_cr3(uintptr_t paddr)
{
	__asm__ volatile("mov %%cr3, %0" : : "r"(paddr) : "memory");
}

uintptr_t mm_read_cr3(void)
{
	uintptr_t p;
	__asm__ volatile("mov %0, %%cr3" : "=r"(p));
	return p;
}

static void mm_flush_tlb(void)
{
	mm_write_cr3(mm_read_cr3());
}

static struct mm_pde *mm_get_pde(const void *vaddr)
{
	return (struct mm_pde *)((uintptr_t)0xfffff000 |
				 (MM_PDE_IDX(vaddr) * sizeof(struct mm_pde)));
}

static struct mm_pte *mm_get_pte(const void *vaddr)
{
	return (struct mm_pte *)((uintptr_t)0xffc00000 |
				 (MM_PDE_IDX(vaddr) << 12) |
				 (MM_PTE_IDX(vaddr) * sizeof(struct mm_pte)));
}

static int mm_map_one(void *vaddr, uintptr_t paddr, u32 flags)
{
	struct mm_pde *pde = mm_get_pde(vaddr);

	if (!pde->present) {
		if (flags & MM_MAP_NOALLOC)
			return -1; // TODO return some error

		size_t pfn = mm_alloc_pageframe(1);
		if (pfn == MM_INVALID_PFN)
			return -1; // TODO return ENOMEM

		pde->present = true;
		pde->pfn = pfn;

		mm_flush_tlb();
	}

	struct mm_pte *pte = mm_get_pte(vaddr);

	memset(pte, 0, sizeof(*pte));

	pte->pfn = MM_PADDR_TO_PFN(paddr);
	pte->rw = !(flags & MM_MAP_RDONLY);
	pte->present = true;

	return 0;
}

static int mm_map(void *vaddr, uintptr_t paddr, size_t length, u32 flags)
{
	u8 *vaddr_c = vaddr;
	u8 *vaddr_c_end = PTR_ALIGN_UP(vaddr_c + length, MM_PAGESIZE);
	vaddr_c = PTR_ALIGN_DOWN(vaddr_c, MM_PAGESIZE);

	for (; vaddr_c != vaddr_c_end;
	     vaddr_c += MM_PAGESIZE, paddr += MM_PAGESIZE) {
		if (mm_map_one(vaddr_c, paddr, flags)) {
			// TODO unmap stuff
			return -1;
		}
	}

	mm_flush_tlb();
	return 0;
}

// **********************************************************************
// Init
// **********************************************************************

static int mm_read_mmap(const struct mb2_info *mb)
{
	const struct mb2_mmap *map =
	    (const struct mb2_mmap *)mb2_find(mb, MB2_TAG_TYPE_MMAP);

	if (!map)
		return -1;

	u32 size = map->base.size - sizeof(map->base) -
		   sizeof(map->entry_size) - sizeof(map->entry_version);

	for (u32 offset = 0; offset < size; offset += map->entry_size) {
		const struct mb2_mmap_entry *entry =
		    (const struct mb2_mmap_entry *)&map->entries[offset];

		if (entry->type == MB2_MMAP_TYPE_AVAIL) {
			size_t pfn_end = MM_PADDR_TO_PFN(PTR_ALIGN_UP(
			    entry->base_addr + entry->length, MM_PAGESIZE));
			size_t pfn_beg = MM_PADDR_TO_PFN(
			    PTR_ALIGN_DOWN(entry->base_addr, MM_PAGESIZE));

			mm_free_pageframe(pfn_beg, pfn_end - pfn_beg);
		}
	}

	return 0;
}

static int mm_init_phys_mem(const struct mb2_info *mb)
{
	if (mm_read_mmap(mb))
		return -1;

	//mark kernel code as used

	size_t pfn_beg = MM_PADDR_TO_PFN(&_kernel_start  - &_kernel_addr);
	//+1 is for the just mapped multiboot struct
	size_t pfn_end = MM_PADDR_TO_PFN(&_kernel_end  - &_kernel_addr) + 1;

	__mm_mark_pageframe(pfn_beg, pfn_end - pfn_beg, true);

	return 0;
}

static const struct mb2_info *mm_map_multiboot(const struct mb2_info *mb,
					       size_t mb_size)
{
	u8 *end = PTR_ALIGN_UP(&_kernel_end, MM_PAGESIZE);

	int rc;
	rc = mm_map(end, (uintptr_t)mb, mb_size, MM_MAP_NOALLOC);

	if (rc)
		return NULL;

	size_t off = mb - PTR_ALIGN_DOWN(mb, MM_PAGESIZE);
	return (struct mb2_info *)end + off;
}

void mm_init(const struct mb2_info *mb, size_t mb_size)
{
	const struct mb2_info *mapped_mb = mm_map_multiboot(mb, mb_size);

	if (!mapped_mb)
		panic(""); // this should probably never happen, unless the
			   // kernel code gets too big

	if (mm_init_phys_mem(mapped_mb))
		panic(""); // did not find any mappings
}

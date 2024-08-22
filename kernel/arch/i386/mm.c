// https://f.osdev.org/viewtopic.php?p=230374&sid=99b22aa6f322a817de79fb61778e78c6#p230374

#include <boot/multiboot2.h>
#include <kernel/arch/i386/mm.h>

#include <kernel/align.h>
#include <kernel/kernel.h>
#include <kernel/types.h>
// TODO rename to libk?
#include <lib/string.h>
#include <stdbool.h>
#include <stddef.h>

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

void mm_flush_tlb(void)
{
	mm_write_cr3(mm_read_cr3());
}

static struct mm_pde *mm_get_pde_at(int pde_idx)
{
	return (struct mm_pde *)((uintptr_t)0xfffff000 |
				 (pde_idx * sizeof(struct mm_pde)));
}

static struct mm_pde *mm_get_pde(const void *vaddr)
{
	return mm_get_pde_at(MM_PDE_IDX(vaddr));
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

static int mm_map(void *vaddr, uintptr_t paddr, size_t len, u32 flags)
{
	u8 *vaddr_c = vaddr;
	u8 *vaddr_c_end = PTR_ALIGN_UP(vaddr_c + len, MM_PAGESIZE);
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

void *mm_find_pages(size_t n)
{
	if (n > 1024)
		return (void*) -1; //currently not supported

	u8* addr = (u8*) &_kernel_addr;
	size_t page = (uintptr_t) addr / MM_PAGESIZE;

	void *best;
	size_t len = 0;

	for (; page < MM_MAX_PAGES; page++, addr += MM_PAGESIZE) {
		struct mm_pde *pde = mm_get_pde(addr);

		if (!pde->present) {
			len = 0;
			continue;
		}

		struct mm_pte *pte = mm_get_pte(addr);
		if (pte->present) {
			len = 0;
			continue;
		}

		if (!len)
			best = addr;

		len += 1;

		if (len == n)
			return best;
	}
	return (void*) -1;
}

void *mm_map_physical(uintptr_t paddr, size_t count, u32 flags)
{
	paddr = PTR_ALIGN_DOWN(paddr, MM_PAGESIZE);

	void *vaddr = mm_find_pages(count);

	if (vaddr == MM_INVALID_PAGE) {
		if (flags & MM_MAP_NOALLOC)
			return vaddr;

		size_t pfn = mm_alloc_pageframe(1);
		if (pfn == MM_INVALID_PFN)
			return MM_INVALID_PAGE;

		int i = 768;
		//TODO don't hardcode
		for (; i < 1024; ++i) {
			struct mm_pde *pde = mm_get_pde_at(i);
			if (pde->present)
				continue;

			pde->pfn = pfn;
			pde->present = 1;
			pde->rw = true;

			vaddr = (void*) ((size_t) i * MM_PAGESIZE * 1024);
			break;
		}

		if (i == 1024)
			return MM_INVALID_PAGE;
	}

	if (mm_map(vaddr, paddr, count * MM_PAGESIZE, flags))
		return NULL;

	return vaddr;
}

// **********************************************************************
// Init
// **********************************************************************

static void mm_read_mmap(const struct mb2_info *info)
{
	const struct mb2_mmap *map =
	    (const struct mb2_mmap *)mb2_find(info, MB2_TAG_TYPE_MMAP);

	if (!map)
		panic(""); // multiboot gave us no information about the memory
			   // map, we can't continue

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
}

static void mm_mark_kernel_code(void)
{
	//mark kernel code as used

	size_t pfn_beg = MM_PADDR_TO_PFN(&_kernel_vstart - &_kernel_addr);
	size_t pfn_end = MM_PADDR_TO_PFN(&_kernel_vend  - &_kernel_addr);

	__mm_mark_pageframe(pfn_beg, pfn_end - pfn_beg, true);
}

static void mm_mark_multiboot(const struct mb2_info *info)
{
	size_t pfn = mm_get_pte(info)->pfn;
	size_t page_count =
	    PTR_ALIGN_UP((u8 *)info + info->total_size, MM_PAGESIZE) -
	    PTR_ALIGN_DOWN((u8 *)info, MM_PAGESIZE);

	__mm_mark_pageframe(pfn, page_count, true);
}

void init_mm(const struct mb2_info *info)
{
	// read available physical memory regions
	mm_read_mmap(info);
	// mark the kernel code in physical memory as unavailable
	mm_mark_kernel_code();
	// temporarily mark the multiboot data as unavailable
	mm_mark_multiboot(info);
}

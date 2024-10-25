#include <kernel/arch/i386/mmu.h>
#include <stddef.h>

#include <kernel/align.h>
#include <kernel/errno.h>
#include <kernel/kernel.h>
#include <kernel/printk.h>
#include <kernel/types.h>
#include <kernel/libc/string.h>
#include <kernel/libc/assert.h>
#include <kernel/arch/i386/platform.h>

static struct page mmu_pages[MMU_MAX_PAGES];

#define MMU_RECURSIVE_PTE_ADDR 0xffc00000
#define MMU_RECURSIVE_PDE_ADDR 0xfffff000

#define MMU_PTE_COUNT 1024
#define MMU_PAGETABLE_SIZE (MMU_PTE_COUNT * sizeof(struct mmu_pte))

void mmu_write_cr3(uintptr_t paddr)
{
	__asm__ volatile("mov %%cr3, %0" : : "r"(paddr) : "memory");
}

uintptr_t mmu_read_cr3(void)
{
	uintptr_t p;
	__asm__ volatile("mov %0, %%cr3" : "=r"(p));
	return p;
}

void mmu_flush_tlb(void)
{
	mmu_write_cr3(mmu_read_cr3());
}

static struct mmu_pte *mmu_pte_at(size_t pde_idx, size_t pte_idx)
{
	return (struct mmu_pte *)((uintptr_t) MMU_RECURSIVE_PTE_ADDR |
				  (pde_idx << MMU_PTE_SHIFT) | (pte_idx * sizeof(struct mmu_pte)));
}

static struct mmu_pte *mmu_vaddr_to_pte(const void *vaddr)
{
	return mmu_pte_at(MMU_PDE_IDX(vaddr), MMU_PTE_IDX(vaddr));
}

static struct mmu_pde *mmu_vaddr_to_pde(const void *vaddr)
{
	return (
	    struct mmu_pde *)(MMU_RECURSIVE_PDE_ADDR |
			      (MMU_PDE_IDX(vaddr) * sizeof(struct mmu_pde)));
}

static struct mmu_pte *mmu_get_pagetable(const struct mmu_pde *pde)
{
	size_t idx = ((uintptr_t)pde & (MMU_PAGESIZE - 1)) / sizeof(*pde);
	return mmu_pte_at(idx, 0);
}


/* atm this just marks the pageframe as used, but in the future(TM) it will
 * increase the reference counting for shared pages etc. */
static void mmu_get_pageframe(struct page *page, size_t nframes)
{
	while (nframes--) {
		atomic_fetch_add_explicit(&page++->refcount, 1, memory_order_relaxed);
	}
}

/* atm this just marks the pageframe as free, but in the future(TM) it will
 * decrease the reference counting for shared pages etc. */
static void mmu_release_pageframe(struct page *page, size_t nframes)
{
	while (nframes--) {
		atomic_fetch_sub_explicit(&page++->refcount, 1, memory_order_relaxed);
	}
}

static bool mmu_page_used(const struct page *page)
{
	return atomic_load_explicit(&page->refcount, memory_order_relaxed) > 0;
}

static struct page *mmu_pfn_to_page(size_t pfn)
{
	return &mmu_pages[pfn];
}

static size_t mmu_page_to_pfn(const struct page *page)
{
	return page - mmu_pages;
}

struct page *mmu_paddr_to_page(uintptr_t paddr)
{
	return mmu_pfn_to_page(MMU_PADDR_TO_PFN(paddr));
}

uintptr_t mmu_page_to_paddr(const struct page *page)
{
	return MMU_PFN_TO_PADDR(mmu_page_to_pfn(page));
}

static struct page *mmu_find_pageframes(uintptr_t hint, size_t nframes)
{
	size_t pfn = MMU_PADDR_TO_PFN(hint);

	size_t best;
	size_t len = 0;

	for (; pfn < MMU_MAX_PAGES; pfn++) {
		if (mmu_page_used(&mmu_pages[pfn])) {
			len = 0;
			continue;
		}

		if (!len)
			best = pfn;

		len += 1;

		if (len >= nframes)
			return mmu_pfn_to_page(best);
	}
	if (hint)
		return mmu_find_pageframes(0, nframes);
	return NULL;
}

struct page *mmu_alloc_pageframe(uintptr_t hint, size_t nframes, u32 flags)
{
	struct page *page;

	if (flags & MMU_ALLOC_FIXED)
		page = mmu_paddr_to_page(hint);
	else
		page = mmu_find_pageframes(hint, nframes);

	if (page)
		mmu_get_pageframe(page, nframes);

	return page;
}

void mmu_free_pageframe(struct page *page, size_t nframes)
{
	mmu_release_pageframe(page, nframes);
}

/* requires tlb flush */
static void mmu_unmap_one(void *vaddr, u32 flags)
{
	struct mmu_pte *pte = mmu_vaddr_to_pte(vaddr);

	if (!pte->present) {
		if (!(flags & MMU_UNMAP_IGNORE_UNMAPPED))
			oops("WARNING: attempt to unmap memory that wasn't mapped");
		return;
	}

	// TODO: zero the pte?
	pte->present = false;
	if (!(flags & MMU_UNMAP_NO_DECR_REF))
		mmu_free_pageframe(mmu_pfn_to_page(pte->pfn), 1);
}

int mmu_unmap(void *vaddr, size_t len, u32 flags)
{
	u8 *vaddr_c = vaddr;
	u8 *vaddr_c_end = PTR_ALIGN_UP(vaddr_c + len, MMU_PAGESIZE);
	vaddr_c = PTR_ALIGN_DOWN(vaddr_c, MMU_PAGESIZE);

	for (; vaddr_c != vaddr_c_end; vaddr_c += MMU_PAGESIZE) {
		mmu_unmap_one(vaddr_c, flags);
	}

	mmu_flush_tlb();
	return 0;
}

static int mmu_alloc_pagetable(struct mmu_pde *pde, int addrspace)
{
	struct page *page =
	    mmu_alloc_pageframe((uintptr_t)&_kernel_pstart, 1, 0);
	if (!page)
		return ENOMEM;

	pde->present = true;
	pde->rw = true;
	pde->pfn = mmu_page_to_pfn(page);
	pde->user = addrspace == MMU_ADDRSPACE_USER;

	mmu_flush_tlb();

	// zero out the just made pagetable

	struct mmu_pte *pte = mmu_get_pagetable(pde);
	memset(pte, 0, MMU_PAGETABLE_SIZE);

	return 0;
}

/* requires tlb flush to take effect */
static int mmu_map_one(void *vaddr, struct page *page, int addrspace, u32 flags)
{
	struct mmu_pde *pde = mmu_vaddr_to_pde(vaddr);

	if (!pde->present) {
		if (flags & MMU_MAP_NOALLOC)
			return ENOMEM;

		int rc = mmu_alloc_pagetable(pde, addrspace);
		if (rc)
			return rc;
	}
	struct mmu_pte *pte = mmu_vaddr_to_pte(vaddr);

	memset(pte, 0, sizeof(*pte));

	pte->pfn = mmu_page_to_pfn(page);
	pte->rw = !(flags & MMU_MAP_RDONLY);
	pte->present = true;
	pte->user = addrspace == MMU_ADDRSPACE_USER;
	return 0;
}

static inline size_t mmu_vaddr_to_pagenum(void *vaddr)
{
	return (uintptr_t)vaddr / MMU_PAGESIZE;
}

static inline void *mmu_pagenum_to_vaddr(size_t pagenum)
{
	return (void *)((uintptr_t)pagenum * MMU_PAGESIZE);
}

static inline bool mmu_pagenum_is_pde(size_t pagenum)
{
	return !(pagenum & 0x3ff);
}

static inline void *mmu_addrspace_beg(int addrspace)
{
	if (addrspace == MMU_ADDRSPACE_KERNEL)
		return (void *)MMU_KERNEL_START;
	return (void *)MMU_USER_START;
}

static inline void *mmu_addrspace_end(int addrspace)
{
	//TODO remove magic values
	if (addrspace == MMU_ADDRSPACE_KERNEL)
		return (void *)0xffffffff;
	return (void *)0xbfffffff;
}

static inline void *mmu_pagenum_to_entry(size_t pagenum)
{
	if (mmu_pagenum_is_pde(pagenum))
		return mmu_vaddr_to_pde(mmu_pagenum_to_vaddr(pagenum));
	return mmu_vaddr_to_pte(mmu_pagenum_to_vaddr(pagenum));
}

static inline struct mmu_pde *mmu_pagenum_to_pde(size_t pagenum)
{
	//TODO remove magic mask 0x3ff
	return mmu_vaddr_to_pde(mmu_pagenum_to_vaddr(pagenum & (~0x3ff)));
}

/* assumption: address spaces are on a 1024 * 4096 boundary */
static void *mmu_find_pages(void *hint, size_t count, int addrspace)
{
	size_t best;
	size_t found = 0;

	size_t start = mmu_vaddr_to_pagenum(mmu_addrspace_beg(addrspace));
	size_t end = mmu_vaddr_to_pagenum(mmu_addrspace_end(addrspace));
	size_t pagenum = hint ? mmu_vaddr_to_pagenum(hint) : start;

	if (!mmu_pagenum_is_pde(pagenum)) {
		struct mmu_pde *pde = mmu_pagenum_to_pde(pagenum);
		if (!pde->present) {
			void *vaddr = mmu_pagenum_to_vaddr(pagenum);
			size_t pte_idx = MMU_PTE_IDX(vaddr);

			size_t nleft = MMU_PTE_COUNT - pte_idx;

			best = pagenum;
			found += nleft;
			pagenum += nleft;
		}
	}

	for (; pagenum <= end; pagenum++) {
		bool is_pde = mmu_pagenum_is_pde(pagenum);

		struct mmu_pte *pte = mmu_pagenum_to_entry(pagenum);
		if (pte->present) {
			found = 0;
			continue;
		}

		if (!found)
			best = pagenum;

		if (is_pde) {
			found += MMU_PTE_COUNT;
			pagenum +=
			    MMU_PTE_COUNT - 1; /* next loop iteration will
						  increment it by one */
		} else {
			found += 1;
		}

		if (found >= count)
			break;
	}

	if (found >= count)
		return mmu_pagenum_to_vaddr(best);

	if (hint)
		return mmu_find_pages(mmu_addrspace_beg(addrspace), count,
				      addrspace);
	return MMU_INVALID_PAGE;
}

/*
 * map a physical address to a vaddr
 */
void *mmu_map_pages(void *vaddr, struct page *frame, size_t nframes,
		    int addrspace, u32 flags)
{
	if (!addrspace) {
		// invalid addrspace
		return MMU_INVALID_PAGE;
	}

	if (vaddr && (flags & MMU_MAP_DOWNWARD)) {
		//TODO assert no overflow
		size_t nbytes = nframes * MMU_PAGESIZE;
		assert((uintptr_t)vaddr >= nbytes);
		vaddr = (u8*) vaddr - nbytes;
	}

	if (!(flags & MMU_MAP_FIXED)) {
		/* vaddr is just a hint, find virtual memory */
		vaddr = mmu_find_pages(vaddr, nframes, addrspace);
		if (vaddr == MMU_INVALID_PAGE)
			return MMU_INVALID_PAGE; /* oom */
	}

	u8 *vaddr_c = vaddr;

	for (size_t i = 0; i < nframes; i++, vaddr_c += MMU_PAGESIZE) {
		int rc = mmu_map_one(vaddr_c, frame + i, addrspace, flags);

		if (rc) {
			mmu_unmap(vaddr, i, 0);
			return MMU_INVALID_PAGE; /* probably oom, check rc */
		}
	}

	mmu_flush_tlb();
	return vaddr;
}

void *mmu_map(void *vaddr, uintptr_t paddr, size_t len, int addrspace, u32 flags)
{
	size_t off = PAGE_OFFSET(paddr);

	uintptr_t pfn_end = ALIGN_UP(paddr + len, MMU_PAGESIZE);
	paddr = ALIGN_DOWN(paddr, MMU_PAGESIZE);

	size_t nframes = (pfn_end - paddr) / MMU_PAGESIZE;

	struct page *page = mmu_paddr_to_page(paddr);

	void *res = mmu_map_pages(vaddr, page, nframes, addrspace, flags);
	if (res == MMU_INVALID_PAGE)
		return res;
	return (u8*) res + off;
}

void *mmu_mmap(void *vaddr, size_t len, int addrspace, u32 flags)
{
	size_t nframes = ALIGN_UP(len, MMU_PAGESIZE) / MMU_PAGESIZE;
	//TODO remove 16*1024*1024 as magic value
	struct page *page = mmu_alloc_pageframe(16 * 1024 * 1024, nframes, 0);
	if (!page)
		return MMU_MAP_FAILED;
	return mmu_map_pages(vaddr, page, nframes, addrspace, flags);
}

static void mmu_invalidate_pages(struct mmu_pde *pde)
{
	if (!pde->present)
		return;

	struct mmu_pte *pte = mmu_get_pagetable(pde);
	for (size_t i = 0; i < 1024; i++) {
		pte[i].present = false;
	}
}

void mmu_invalidate_user(void)
{
	for (size_t pfn = 0; pfn < MMU_PADDR_TO_PFN(MMU_KERNEL_START); pfn += 1024) {
		struct mmu_pde *pde = mmu_pagenum_to_pde(pfn);
		mmu_invalidate_pages(pde);
	}
	mmu_flush_tlb();
}

static enum irq_result mmu_on_pagefault(u8 irq, struct cpu_state *state, void *ctx)
{
	(void) irq;
	struct pagefault fault = {
		.is_write = state->err_code & 0x02,
		.is_present = state->err_code & 0x01,
		.addr = 0,
	};

	__asm__ volatile ("mov %0, cr2" : "=r"(fault.addr));
	enum irq_result (*handler)(struct cpu_state*, const struct pagefault*) = ctx;
	return handler(state, &fault);
}

int mmu_register_pagefault_handler(enum irq_result (*handler)(
    struct cpu_state *state, const struct pagefault *info))
{
	return irq_register_handler(IRQ_PAGEFAULT, mmu_on_pagefault, handler);
}

static void mmu_read_mmap(const struct mb2_info *info)
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
			size_t pfn_end = MMU_PADDR_TO_PFN(PTR_ALIGN_UP(
			    entry->base_addr + entry->length, MMU_PAGESIZE));
			size_t pfn_beg = MMU_PADDR_TO_PFN(
			    PTR_ALIGN_DOWN(entry->base_addr, MMU_PAGESIZE));

			struct page *page = mmu_pfn_to_page(pfn_beg);

			mmu_free_pageframe(page, pfn_end - pfn_beg);
		}
	}
}

static void mmu_mark_kernel_code(void)
{
	// mark kernel code as used

	size_t pfn_beg = MMU_PADDR_TO_PFN(PTR_ALIGN_DOWN(&_kernel_vstart, MMU_PAGESIZE) - &_kernel_addr);
	size_t pfn_end = MMU_PADDR_TO_PFN(PTR_ALIGN_UP(&_kernel_vend, MMU_PAGESIZE) - &_kernel_addr);

	mmu_alloc_pageframe(MMU_PFN_TO_PADDR(pfn_beg), pfn_end - pfn_beg,
			    MMU_ALLOC_FIXED);
}

static void mmu_mark_multiboot(const struct mb2_info *info)
{
	size_t pfn = mmu_vaddr_to_pte(info)->pfn;
	size_t page_count =
	    PTR_ALIGN_UP((u8 *)info + info->total_size, MMU_PAGESIZE) -
	    PTR_ALIGN_DOWN((u8 *)info, MMU_PAGESIZE);

	mmu_alloc_pageframe(MMU_PFN_TO_PADDR(pfn), page_count, MMU_ALLOC_FIXED);
}

void init_mmu(void)
{
	const struct mb2_info *info = mb2_get_info();
	// read available physical memory regions
	mmu_read_mmap(info);
	// mark the kernel code in physical memory as unavailable
	mmu_mark_kernel_code();
	// temporarily mark the multiboot data as unavailable
	mmu_mark_multiboot(info);
}

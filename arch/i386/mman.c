#include "multiboot2.h"
#include <kernel/kernel.h>
#include <kernel/types.h>
#include <stdbool.h>
#include <stddef.h>

#define PM_PAGESIZE 0x1000
#define PM_MAX_PAGES (1024 * 1024)

#define MM_ALIGN_DOWN(x, boundary) \
	_Generic((x), void *: ((uintptr_t)(x) & (~(((size_t)boundary) - 1))))
#define MM_ALIGN_UP(x, boundary) \
	_Generic((x),            \
	    size_t: (((x) + ((boundary) - 1)) & (~((size_t)(boundary) - 1))))

// A bitmap is used to keep track of which physical pages are used. 0 means
// used, 1 means free.
//
// The reason for using 0 for meaning "used" is so that instead of, during
// initialization, marking unusable pages as "used" we only mark pages that are
// actually available

static struct mm_state {
	u32 pagetable[];
} mm_state;
static u8 page_bitmap[0x20000];

extern u8 _kernel_start, _kernel_end;

void free_page(void *paddr, size_t size);

// TODO WRONG!
static size_t __pm_pageidx(const void *addr)
{
	return (uintptr_t)addr >> 15;
}

static size_t __pm_pageoff(const void *addr)
{
	return ((uintptr_t)addr >> 12) & 7;
}

static void *__pm_page_to_addr(size_t pagenum)
{
	return (void *)((uintptr_t)pagenum * PM_PAGESIZE);
}

static void __pm_mark_page(void *addr, bool used)
{
	size_t idx = __pm_pageidx(addr);
	size_t off = __pm_pageoff(addr);

	page_bitmap[idx] &= ~(1 << off);
	page_bitmap[idx] |= !used * (1 << off);
}

static bool __pm_is_used(size_t pagenum)
{
	size_t idx = pagenum >> 3;
	size_t off = pagenum & 7;

	return page_bitmap[idx] & (1 << off);
}

static void __pm_mark_pages(void *paddr, size_t size, bool used)
{
	u8 *addr = (u8 *)MM_ALIGN_DOWN(paddr, PM_PAGESIZE);
	u8 *end = addr + size;

	for (; addr < end; addr += PM_PAGESIZE) {
		__pm_mark_page(addr, used);
	}
}

static void __pm_init(const struct mb_mmap *map)
{
	u32 size = map->base.size - sizeof(map->base) -
		   sizeof(map->entry_size) - sizeof(map->entry_version);

	for (u32 offset = 0; offset < size; offset += map->entry_size) {
		const struct mb_mmap_entry *entry =
		    (const struct mb_mmap_entry *)&map->entries[offset];

		if (entry->type == MB_MMAP_TYPE_AVAIL)
			free_page((void *)(uintptr_t)entry->base_addr,
				  entry->length);
	}

	// Make sure we don't reuse kernel memory
	__pm_mark_pages(&_kernel_start, &_kernel_end - &_kernel_start, true);
}

int pm_getpagesize(void)
{
	return 0x1000;
}

void pm_init(const struct mb_info *data)
{
	const struct mb_tag_base *cur =
	    (const struct mb_tag_base *)(void *)data->tags;

	for (; cur->type != MB_TAG_TYPE_END; cur = MB_NEXT_TAG(cur)) {
		if (cur->type != MB_TAG_TYPE_MMAP)
			continue;

		__pm_init((const struct mb_mmap *)cur);
		return;
	}
	panic(""); // no memory map found
}

void *alloc_page(size_t size)
{
	size_t idx = 0;
	size_t len = 0;

	size = MM_ALIGN_UP(size, PM_PAGESIZE);

	size_t req_count = size / PM_PAGESIZE;

	for (size_t cur = 0; cur < PM_MAX_PAGES; cur++) {
		if (__pm_is_used(idx)) {
			len = 0;
			continue;
		}

		if (!len)
			idx = cur;

		len += 1;

		if (len >= req_count) {
			void *paddr = __pm_page_to_addr(idx);
			__pm_mark_pages(paddr, size, true);
			return paddr;
		}
	}
	return NULL;
}

void free_page(void *paddr, size_t size)
{
	__pm_mark_pages(paddr, size, false);
}

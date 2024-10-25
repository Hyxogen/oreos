#include <kernel/malloc/ma/internal.h>

#include <kernel/mmu.h>
#include <kernel/libc/string.h>

int ma_sysalloc_granularity(void)
{
	return MMU_PAGESIZE;
}

void *ma_sysalloc(size_t size)
{
	size_t nframes = size / ma_sysalloc_granularity();
	struct page *page = mmu_alloc_pageframe(MMU_PAGEFRAME_HINT, nframes, 0);
	if (!page)
		return MA_SYSALLOC_FAILED;

	void *p = mmu_map_pages(NULL, page, nframes, MMU_ADDRSPACE_KERNEL, MMU_MAP_NO_INCR_REF);
	if (p == (void*) -1) //TODO use MMU_INVALID_PAGE
		return MA_SYSALLOC_FAILED;

	memset(p, 0, nframes * ma_sysalloc_granularity());
	return p;
}

bool ma_sysfree(void *p, size_t size) { return mmu_unmap(p, size, 0) == 0; }

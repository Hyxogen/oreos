#include <kernel/malloc/ma/internal.h>

#include <kernel/mmu.h>
#include <lib/string.h>

int ma_sysalloc_granularity(void)
{
	return MMU_PAGESIZE;
}

void *ma_sysalloc(size_t size)
{
	size_t nframes = size / ma_sysalloc_granularity();
	struct page *page = mmu_alloc_pageframe(16 * 1024 * 1024, nframes, 0);

	void *p = mmu_map_pages(NULL, page, nframes, MMU_ADDRSPACE_KERNEL, 0);
	if (p == (void*) -1) //TODO use MMU_INVALID_PAGE
		return MA_SYSALLOC_FAILED;

	memset(p, 0, size);
	return p;
}

bool ma_sysfree(void *p, size_t size) { return mmu_unmap(p, size) == 0; }

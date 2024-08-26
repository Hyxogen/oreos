#include <kernel/malloc/ma/internal.h>

void *kmalloc(size_t n) { return ma_malloc(n); }
void *kcalloc(size_t nmemb, size_t size) { return ma_calloc(nmemb, size); }
void *krealloc(void *p, size_t newsize) { return ma_realloc(p, newsize); }
void kfree(void *p) { ma_free(p); }

void *kaligned_alloc(size_t align, size_t size)
{
	return ma_aligned_alloc(align, size);
}
size_t kmalloc_usable_size(void *p) { return ma_malloc_usable_size(p); };
void *kmemalign(size_t align, size_t size) { return ma_memalign(align, size); }
void *kvalloc(size_t size) { return ma_valloc(size); }
void *kpvalloc(size_t size) { return ma_pvalloc(size); }
int kposix_memalign(void **memptr, size_t align, size_t size)
{
	return ma_posix_memalign(memptr, align, size);
}

#include <kernel/sched.h>
#include <kernel/align.h>
#include <kernel/mmu.h>
#include <stdbool.h>
#include <kernel/libc/assert.h>
#include <kernel/errno.h>
#include <kernel/malloc/malloc.h>
#include <kernel/libc/string.h>
#include <kernel/irq.h>
#include <kernel/printk.h>
#include <kernel/kernel.h>

/* TODO REMOVE */
#include <kernel/debug.h>
/* checks if [a_beg, a_end) overlaps with [b_beg, b_end) */
static bool overlaps(uintptr_t a_beg, uintptr_t a_end, uintptr_t b_beg,
		     uintptr_t b_end)
{
	if (a_beg > a_end)
		return overlaps(a_end, a_beg, b_beg, b_end);
	if (b_beg > b_end)
		return overlaps(a_beg, a_end, b_end, b_beg);
	if (b_beg < a_beg)
		return overlaps(b_beg, b_end, a_beg, a_end);
	return a_end > b_beg;
}

static bool is_subspace(uintptr_t super_beg, uintptr_t super_end, uintptr_t sub_beg, uintptr_t sub_end)
{
	return sub_beg >= super_beg && sub_end <= super_end;
}

bool vma_is_mapped(const struct vma_area *area, uintptr_t start, uintptr_t end)
{
	if (!area)
		return false;

	if (overlaps(area->start, area->end, start, end))
		return true;
	if (start < area->start)
		return vma_is_mapped(area->left, start, end);
	return vma_is_mapped(area->right, start, end);
}

static struct vma_area *vma_create_area(uintptr_t start, uintptr_t end, u32 flags)
{
	struct vma_area *node = kmalloc(sizeof(*node));
	if (!node)
		return node;

	size_t npages = (end - start) / MMU_PAGESIZE;

	node->pages = kcalloc(sizeof(*node->pages), npages);
	if (!node->pages) {
		kfree(node);
		return NULL;
	}

	node->parent = NULL;
	node->left = NULL;
	node->right = NULL;
	node->start = start;
	node->end = end;
	node->flags = flags;
	return node;
}

static void vma_free_area(struct vma_area *area)
{
	memset(area, 0, sizeof(*area));
	kfree(area);
}

static void vma_insert(struct vma_area **tree, struct vma_area *node)
{
	if (!*tree) {
		*tree = node;
	} else {
		node->parent = *tree;
		if (node->start < (*tree)->start)
			return vma_insert(&(*tree)->left, node);
		return vma_insert(&(*tree)->right, node);
	}
}

static void vma_remove(struct vma_area **tree, struct vma_area *node)
{
	if (node->parent) {
		if (node->parent->left == node) {
			node->parent->left = NULL;
		} else {
			assert(node->parent->right);
			node->parent->right = NULL;
		}
	} else {
		assert(*tree == node);
		*tree = NULL;
	}

	if (node->left && node->right) {
		vma_insert(&node->right, node->left);
		vma_insert(tree, node->right);
	} else if (node->left || node->right) {
		struct vma_area *tmp = node->left ? node->left : node->right; 
		vma_insert(tree, tmp);
	}

	vma_free_area(node);
}

static int vma_add_mapping(struct vma_area **area, uintptr_t start, uintptr_t end, u32 flags)
{
	//TODO join ranges that are the same
	struct vma_area *node = vma_create_area(start, end, flags);
	if (!node)
		return -ENOMEM;

	vma_insert(area, node);
	return 0;
}

static uintptr_t vma_find_free(const struct vma_area *area, uintptr_t start, size_t len)
{
	if (!area) {
		if (start + len < start)
			return -1;
		if (start >= MMU_KERNEL_START || start + len >= MMU_KERNEL_START)
			return -1;
		return start;
	}
	return vma_find_free(area->right, area->end, len);
}

int vma_map(struct mm *mm, uintptr_t *addr, size_t len, u32 flags)
{
	if (flags & VMA_MAP_FIXED_NOREPLACE || (addr && *addr)) {
		assert(addr);

		uintptr_t start = ALIGN_DOWN(*addr, MMU_PAGESIZE);
		uintptr_t end = ALIGN_UP(*addr + len, MMU_PAGESIZE);

		if (vma_is_mapped(mm->root, start, end)) {
			if (flags & VMA_MAP_FIXED_NOREPLACE)
				return -EEXIST;

			*addr = 0;
			return vma_map(mm, addr, len, flags);
		}

		return vma_add_mapping(&mm->root, start, end, flags);
	}

	len = ALIGN_UP(len, MMU_PAGESIZE);
	*addr = vma_find_free(mm->root, MMU_USER_START, len);
	assert(IS_ALIGNED(*addr, MMU_PAGESIZE));

	if (*addr != (uintptr_t) -1)
		return vma_add_mapping(&mm->root, *addr, *addr + len, flags);
	return -ENOMEM;
}

struct vma_area *vma_find_mapping(const struct vma_area *area, uintptr_t start,
				uintptr_t end)
{
	if (!area)
		return NULL;
	if (is_subspace(area->start, area->end, start, end))
		return (struct vma_area*) area;
	if (start < area->start)
		return vma_find_mapping(area->left, start, end);
	return vma_find_mapping(area->right, start, end);
}

static int vma_unmap_from_area(struct mm *mm, struct vma_area *area, uintptr_t start,
			  uintptr_t end)
{
	assert(is_subspace(area->start, area->end, start, end));

	if (!area)
		return -ENOENT;
	if (area->start < start)
		start = area->start;
	if (area->end > end)
		end = area->end;

	//TODO mmu_unmap stuff to actually free physical memory
	if (area->start == start && area->end == end) {
		vma_remove(&mm->root, area);
	} else if (area->start == start) {
		area->start = end;
	} else if (area->end == end) {
		area->end = start;
	} else {
		struct vma_area *node = vma_create_area(end, area->end, area->flags);
		if (!node)
			return -ENOMEM;
		area->end = start;

		node->right = area->right;
		node->right->parent = node;

		area->right = node;
		node->parent = area;
	}
	mmu_unmap((void*) start, end - start, MMU_UNMAP_IGNORE_UNMAPPED);
	return 0;
}

//         [20-30)
// [10-20)         [
int vma_unmap(struct mm *mm, uintptr_t addr, size_t len)
{
	uintptr_t start = ALIGN_DOWN(addr, MMU_PAGESIZE);
	uintptr_t end = ALIGN_UP(addr + len, MMU_PAGESIZE);

	struct vma_area *area = vma_find_mapping(mm->root, start, end);
	return vma_unmap_from_area(mm, area, start, end);
}

static size_t vma_get_page_idx(const struct vma_area *area, uintptr_t addr)
{
	assert(IS_ALIGNED(addr, MMU_PAGESIZE));
	return (addr - area->start) / MMU_PAGESIZE;
}

int vma_map_now_one(struct vma_area *area, uintptr_t addr)
{
	size_t idx = vma_get_page_idx(area, addr);

	/* TODO set permissions correctly */
	bool readonly = !(area->flags & VMA_MAP_PROT_WRITE) || (area->flags & VMA_MAP_COW);

	if (area->pages[idx]) {
		void *res = mmu_map_pages((void *)addr, area->pages[idx], 1,
					  MMU_ADDRSPACE_USER,
					  MMU_MAP_FIXED | MMU_MAP_NO_INCR_REF |
					      (MMU_MAP_RDONLY * readonly));
		if (res == MMU_MAP_FAILED)
			return -ENOMEM;
		return 0;
	}

	area->pages[idx] = mmu_alloc_pageframe(MMU_PAGEFRAME_HINT, 1, 0);
	if (!area->pages[idx])
		return -ENOMEM;

	/*void *tmp = mmu_map_pages(NULL, area->pages[idx], 1, MMU_ADDRSPACE_KERNEL, 0);
	if (tmp == MMU_MAP_FAILED)
		return -ENOMEM;

	memset(tmp, 0, MMU_PAGESIZE);
	mmu_unmap(tmp, MMU_PAGESIZE, 0);*/

	int res = vma_map_now_one(area, addr);
	return res;
}

int vma_map_now(struct mm *mm, uintptr_t addr, size_t len)
{
	uintptr_t start = ALIGN_DOWN(addr, MMU_PAGESIZE);
	uintptr_t end = ALIGN_UP(addr + len, MMU_PAGESIZE);

	for (; start != end; start += MMU_PAGESIZE) {
		struct vma_area *area = vma_find_mapping(mm->root, start, end);
		if (!area)
			return -EINVAL;

		int res = vma_map_now_one(area, start);
		if (res)
			return res;
	}
	return 0;
}

static void vma_unmap_all(struct mm *mm, struct vma_area *area)
{
	if (!area)
		return;
	vma_unmap_all(mm, area->left);
	vma_unmap_all(mm, area->right);
	assert(!vma_unmap_from_area(mm, area, area->start, area->end));
}

int vma_destroy(struct mm *mm)
{
	vma_unmap_all(mm, mm->root);
	return 0;
}

int vma_do_cow_one(struct vma_area *area, uintptr_t addr)
{
	assert(IS_ALIGNED(addr, MMU_PAGESIZE));

	size_t idx = vma_get_page_idx(area, addr);
	/* TODO we don't need alloc a new page if the refcount is back to 1 */
	struct page *src = area->pages[idx];
	assert(src);

	void *src_vaddr = mmu_map_pages(NULL, src, 1, MMU_ADDRSPACE_KERNEL, 0);
	if (src_vaddr == MMU_MAP_FAILED)
		return -ENOMEM;

	struct page *dest = mmu_alloc_pageframe(MMU_PAGEFRAME_HINT, 1, 0);
	if (!dest) {
		mmu_unmap(src_vaddr, MMU_PAGESIZE, 0);
		return -ENOMEM;
	}

	void *dest_vaddr = mmu_map_pages(NULL, dest, 1, MMU_ADDRSPACE_KERNEL, 0);
	if (dest_vaddr == MMU_MAP_FAILED) {
		mmu_free_pageframe(dest, 1);
		mmu_unmap(src_vaddr, MMU_PAGESIZE, 0);
		return -ENOMEM;
	}

	memcpy(dest_vaddr, src_vaddr, MMU_PAGESIZE);

	mmu_unmap(src_vaddr, MMU_PAGESIZE, 0);
	mmu_unmap(dest_vaddr, MMU_PAGESIZE, 0);
	mmu_free_pageframe(src, 1);

	area->pages[idx] = dest;
	area->flags &= ~VMA_MAP_COW;
	return vma_map_now_one(area, addr);
}

static int vma_clone_one(struct mm *dest, struct vma_area *area)
{
	if (!area)
		return 0;

	int res = vma_clone_one(dest, area->left);
	if (res)
		return res;
	res = vma_clone_one(dest, area->left);
	if (res)
		return res;

	u32 flags_saved = area->flags;

	area->flags |= VMA_MAP_COW;
	size_t nframes = (area->end - area->start) / MMU_PAGESIZE;
	struct vma_area *node = vma_create_area(area->start, area->end, area->flags);
	if (!node)
		goto error;
	node->pages = kmalloc(nframes * sizeof(*node->pages));
	if (!node->pages) {
		/* TODO make vma_destroy_area function */
		kfree(node);
		goto error;
	}

	for (size_t i = 0; i < nframes; i++) {
		node->pages[i] = area->pages[i];
		if (node->pages[i])
			mmu_page_acquire(node->pages[i]);
	}

	vma_insert(&dest->root, node);
	return 0;
error:
	area->flags = flags_saved;
	return -ENOMEM;
}

int vma_clone(struct mm *dest, struct mm *src)
{
	dest->root = NULL;
	return vma_clone_one(dest, src->root);
}

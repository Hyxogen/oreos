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

	node->page = NULL;
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

//         [20-30)
// [10-20)         [
int vma_unmap(struct mm *mm, uintptr_t addr, size_t len)
{
	uintptr_t start = ALIGN_DOWN(addr, MMU_PAGESIZE);
	uintptr_t end = ALIGN_UP(addr + len, MMU_PAGESIZE);

	struct vma_area *area = vma_find_mapping(mm->root, start, end);

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

int vma_map_now(struct vma_area *area)
{
	size_t nframes = (area->end - area->start) / MMU_PAGESIZE;
	if (area->page) {
		void *res = mmu_map_pages((void *)area->start, area->page, nframes,
			      MMU_ADDRSPACE_USER, MMU_MAP_FIXED);
		if (res == MMU_MAP_FAILED)
			return -ENOMEM;
		return 0;
	}

	area->page = mmu_alloc_pageframe(16 * 1024 * 1024, nframes, 0);
	if (!area->page)
		return -ENOMEM;

	int res = vma_map_now(area);
	if (!res)
		memset((void*) area->start, 0, area->end - area->start);
	return res;
}

static enum irq_result pagefault_handler(struct cpu_state *state,
					 const struct pagefault *fault)
{
	if (!is_from_userspace(state) && !is_from_uaccess(state))
		return IRQ_PANIC;

	struct process *proc = sched_get_current_proc();
	if (!proc)
		return IRQ_PANIC;

	uintptr_t aligned = ALIGN_DOWN(fault->addr, MMU_PAGESIZE);

	struct vma_area *area =
	    vma_find_mapping(proc->mm.root, aligned, aligned + MMU_PAGESIZE);

	if (!area) {
		printk("TMP: irq stackstrace:\n");
		dump_stacktrace_at(state);
		printk("cpu state:\n");
		dump_state(state);
		panic("page fault");
	}

	if (!area || (fault->is_write && !(area->flags & VMA_MAP_PROT_WRITE)) ||
	    (!fault->is_write && !(area->flags & VMA_MAP_PROT_READ))) {
		assert(0);
		goto kill_proc;
	}
	/* TODO COW */

	/* TODO set proper read/write access */
	if (vma_map_now(area)) {
		/* TODO we are out of mem, do something smart */
		goto kill_proc;
	}

	proc_release(proc);
	return IRQ_CONTINUE;
kill_proc:
	sched_kill(proc, -1);
	proc_release(proc);
	sched_yield(state);
}

void init_vma(void)
{
	mmu_register_pagefault_handler(pagefault_handler);
}

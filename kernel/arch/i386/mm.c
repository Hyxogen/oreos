#include <kernel/sched.h>
#include <kernel/align.h>
#include <kernel/mmu.h>
#include <stdbool.h>
#include <kernel/libc/assert.h>
#include <kernel/errno.h>
#include <kernel/malloc/malloc.h>
#include <kernel/libc/string.h>
#include <kernel/arch/i386/platform.h>
#include <kernel/irq.h>
#include <kernel/printk.h>
#include <kernel/kernel.h>
//TODO remove
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

bool mm_is_mapped(const struct mm_area *area, uintptr_t start, uintptr_t end)
{
	if (!area)
		return false;

	if (overlaps(area->start, area->end, start, end))
		return true;
	if (start < area->start)
		return mm_is_mapped(area->left, start, end);
	return mm_is_mapped(area->right, start, end);
}

static struct mm_area *mm_create_area(uintptr_t start, uintptr_t end, u32 flags)
{
	struct mm_area *node = kmalloc(sizeof(*node));
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

static void mm_free_area(struct mm_area *area)
{
	memset(area, 0, sizeof(*area));
	kfree(area);
}

static void mm_insert(struct mm_area **tree, struct mm_area *node)
{
	if (!*tree) {
		*tree = node;
	} else {
		node->parent = *tree;
		if (node->start < (*tree)->start)
			return mm_insert(&(*tree)->left, node);
		return mm_insert(&(*tree)->right, node);
	}
}

static void mm_remove(struct mm_area **tree, struct mm_area *node)
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
		mm_insert(&node->right, node->left);
		mm_insert(tree, node->right);
	} else if (node->left || node->right) {
		struct mm_area *tmp = node->left ? node->left : node->right; 
		mm_insert(tree, tmp);
	}

	mm_free_area(node);
}

static int mm_add_mapping(struct mm_area **area, uintptr_t start, uintptr_t end, u32 flags)
{
	//TODO join ranges that are the same
	struct mm_area *node = mm_create_area(start, end, flags);
	if (!node)
		return -ENOMEM;

	mm_insert(area, node);
	return 0;
}

static uintptr_t mm_find_free(const struct mm_area *area, uintptr_t start, size_t len)
{
	if (!area) {
		if (start + len < start)
			return -1;
		if (start >= MMU_KERNEL_START || start + len >= MMU_KERNEL_START)
			return -1;
		return start;
	}
	return mm_find_free(area->right, area->end, len);
}

int mm_map(struct mm *mm, uintptr_t *addr, size_t len, u32 flags)
{
	if (addr && *addr) {
		uintptr_t start = ALIGN_DOWN(*addr, MMU_PAGESIZE);
		uintptr_t end = ALIGN_UP(*addr + len, MMU_PAGESIZE);

		if (mm_is_mapped(mm->root, start, end)) {
			*addr = 0;
			return mm_map(mm, addr, len, flags);
		}

		return mm_add_mapping(&mm->root, start, end, flags);
	}

	len = ALIGN_UP(len, MMU_PAGESIZE);
	*addr = mm_find_free(mm->root, MMU_USER_START, len);
	assert(IS_ALIGNED(*addr, MMU_PAGESIZE));

	if (*addr != (uintptr_t) -1)
		return mm_add_mapping(&mm->root, *addr, *addr + len, flags);
	return -ENOMEM;
}

struct mm_area *mm_find_mapping(const struct mm_area *area, uintptr_t start,
				uintptr_t end)
{
	if (!area)
		return NULL;
	if (is_subspace(area->start, area->end, start, end))
		return (struct mm_area*) area;
	if (area->start < start)
		return mm_find_mapping(area->left, start, end);
	return mm_find_mapping(area->right, start, end);
}

//         [20-30)
// [10-20)         [
int mm_unmap(struct mm *mm, uintptr_t addr, size_t len)
{
	uintptr_t start = ALIGN_DOWN(addr, MMU_PAGESIZE);
	uintptr_t end = ALIGN_UP(addr + len, MMU_PAGESIZE);

	struct mm_area *area = mm_find_mapping(mm->root, start, end);

	if (!area)
		return -ENOENT;
	if (area->start < start)
		start = area->start;
	if (area->end > end)
		end = area->end;

	//TODO mmu_unmap stuff to actually free physical memory
	if (area->start == start && area->end == end) {
		mm_remove(&mm->root, area);
	} else if (area->start == start) {
		area->start = end;
	} else if (area->end == end) {
		area->end = start;
	} else {
		struct mm_area *node = mm_create_area(end, area->end, area->flags);
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

static uintptr_t get_pagefault_addr(void)
{
	uintptr_t addr;

	__asm__ volatile("mov %0, cr2" : "=r"(addr));
	return addr;
}

int mm_map_now(struct mm_area *area)
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

	int res = mm_map_now(area);
	if (!res)
		memset((void*) area->start, 0, area->end - area->start);
	return res;
}

static enum irq_result pagefault_handler(u8 irq, struct cpu_state *state,
					 void *dummy)
{
	(void)irq;
	(void)dummy;

	if (!is_userspace(state)) {
		return IRQ_PANIC;
	} 

	bool saved = sched_set_preemption(false);
	uintptr_t addr = get_pagefault_addr();
	struct process *proc = sched_cur();
	assert(proc);

	struct mm_area *area = mm_find_mapping(proc->mm.root, addr, MMU_PAGESIZE);
	if (!area || state->err_code & 0x02) {
		/* TODO detect COW */
		proc->status = DEAD;
		proc->status = -1; /* TODO set proper status */
		sched_preempt(state);
	}

	BOCHS_BREAK;
	/* TODO set proper read/write access */
	if (mm_map_now(area)) {
		/* TODO we are out of mem, do something smart */
		proc->status = DEAD;
		proc->status = -1; /* TODO set proper status */
		sched_preempt(state);
	}
	sched_set_preemption(saved);
	return IRQ_CONTINUE;
}

void init_mm(void)
{
	irq_register_handler(IRQ_PAGEFAULT, pagefault_handler, NULL);
}

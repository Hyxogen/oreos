#include <stdbool.h>
#include <kernel/mmu.h>
#include <kernel/align.h>
#include <kernel/irq.h>
#include <kernel/sched.h>
#include <kernel/libc/assert.h>
#include <kernel/arch/i386/platform.h>
#include <kernel/printk.h>
#include <kernel/kernel.h>
#include <kernel/signal.h>

static uintptr_t get_pagefault_addr(void)
{
	uintptr_t res = -1;
	__asm__ volatile ("mov %0, cr2" : "=r"(res));
	return res;
}

static bool is_write(const struct cpu_state *state)
{
	return state->err_code & 0x02;
}

static bool is_present(const struct cpu_state *state)
{
	return state->err_code & 0x01;
}

/* XXX seems to work, but probably requires more testing */
static void pop_frame(struct cpu_state *state)
{
	u32* frame_start = (void*) (uintptr_t) state->ebp;

	/*
	 * ...
	 * [previous function stack]
	 * [return address]
	 * [previous ebp]
	 */
	u32 saved_ebp = *frame_start;
	u32 retaddr = *(frame_start + 1);
	u32 stack = (uintptr_t) (frame_start + 2);

	state->eip = retaddr;
	state->ebp = saved_ebp;
	state->esp = stack;
}

static void fail_uaccess(struct cpu_state *state)
{
	pop_frame(state);
	state->eax = -1;
}

static void maybe_fail_uaccess(struct cpu_state *state)
{
	if (is_from_uaccess(state))
		fail_uaccess(state);
}

static enum irq_result do_page_fault(u8 irq, struct cpu_state *state, void *dummy)
{
	(void) irq;
	(void) dummy;

	if (!is_from_userspace(state) && !is_from_uaccess(state))
		panic("pagefault from the kernel");

	struct process *proc = sched_get_current_proc();
	if (!proc)
		panic("no process");

	uintptr_t addr = ALIGN_DOWN(get_pagefault_addr(), MMU_PAGESIZE);

	/* TODO we need to lock the mm struct */
	struct vma_area *area = vma_find_mapping(proc->mm.root, addr, addr + MMU_PAGESIZE);
	if (!area || (is_write(state) && !(area->flags & VMA_MAP_PROT_WRITE))) {
		sched_signal(proc, SIGSEGV);
		maybe_fail_uaccess(state);
	} else if (is_write(state) && is_present(state)) {
		if (vma_do_cow_one(area, addr)) {
			assert(0);
			sched_signal(proc, SIGSEGV);
			maybe_fail_uaccess(state);
		} 
	} else {
		if (vma_map_now_one(area, addr)) {
			assert(0);
			sched_signal(proc, SIGSEGV); /* TODO this should not be a segfault */
			maybe_fail_uaccess(state);
		}
	}

	proc_release(proc);
	return IRQ_CONTINUE;
}

void mmu_init_pagefault_handler(void)
{
	int res = irq_register_handler(IRQ_PAGEFAULT, do_page_fault, NULL);
	assert(!res);
}

#include <kernel/kernel.h>
#include <kernel/printk.h>
#include <kernel/sched.h>
#include <kernel/malloc/malloc.h>
#include <kernel/arch/i386/platform.h>
#include <kernel/arch/i386/mmu.h>
#include <kernel/libc/string.h>
#include <kernel/errno.h>
#include <kernel/irq.h>

#include <kernel/debug.h>

u32 _get_eflags(void);

static void *push(void *stackp, u32 v)
{
	u32* stack = stackp;
	*--stack = v;
	return stack;
}

static void proc_get_selectors(int ring, u16 *code_sel, u16 *data_sel)
{
	if (ring == 0) {
		*code_sel = I386_KERNEL_CODE_SELECTOR | 0;
		*data_sel = I386_KERNEL_DATA_SELECTOR | 0;
	} else if (ring == 3) {
		*code_sel = I386_USER_CODE_SELECTOR | 3;
		*data_sel = I386_USER_DATA_SELECTOR | 3;
	} else {
		panic("invalid ring");
	}
}

static u32 proc_get_eflags(int ring)
{
	/* eflags: IOPL 3 (0x3000) IF: (0x0200) legacy: (0x0002) */
	//TODO remove magic values
	if (ring == 0)
		return 0x0202;
	else if (ring == 3)
		return 0x3202;
	panic("proc_get_eflags: invalid ring");
}

struct process *proc_create(void *start, u32 flags)
{
	struct process *proc = kmalloc(sizeof(*proc));
	if (!proc)
		return proc;
	proc->kernel_stack = NULL;
	proc->context = NULL;

	int ring;

	if (flags & PROC_FLAG_RING0) {
		ring = 0;
	} else if (flags & PROC_FLAG_RING3) {
		ring = 3;
	} else {
		printk("ring not specified\n");
		goto err;
	}

	u16 code_sel, data_sel;
	proc_get_selectors(ring, &code_sel, &data_sel);
	u32 eflags = proc_get_eflags(ring);


	//TODO don't hardcode kernel stack size
	proc->kernel_stack = mmu_mmap(NULL, 0x4000, MMU_ADDRSPACE_KERNEL, 0);
	if (proc->kernel_stack == MMU_MAP_FAILED)
		goto err;

	proc->kernel_stack = (u8*)proc->kernel_stack + 0x4000;
	void *top = proc->kernel_stack;

	u32 esp = 0;
	if (ring == 0)
		esp = (uintptr_t) proc->kernel_stack;

	top = push(top, data_sel); /* ss */
	top = push(top, esp); /* esp */
	top = push(top, eflags); /* eflags */
	top = push(top, code_sel); /* cs */
	top = push(top, (uintptr_t)start); /* eip */
	top = push(top, 0); /* err_code */
	top = push(top, 0); /* vec_num */
	top = push(top, 0); /* eax */
	top = push(top, 0); /* ecx */
	top = push(top, 0); /* edx */
	top = push(top, 0); /* ebx */
	top = push(top, 0); /* old_esp */
	top = push(top, 0); /* ebp */
	top = push(top, 0); /* esi */
	top = push(top, 0); /* edi */
	top = push(top, data_sel); /* ds */

	proc->context = top;

	proc->pid = -1;
	proc->status = DEAD;
	proc->next = NULL;

	return proc;
err:
	if (proc->kernel_stack)
		mmu_unmap(proc->kernel_stack, 0x4000);
	kfree(proc->context);
	kfree(proc);
	return NULL;
}

void proc_prepare_switch(struct process *proc)
{
	disable_irqs();
	_tss.esp0 = (uintptr_t) proc->kernel_stack;
	enable_irqs();
}

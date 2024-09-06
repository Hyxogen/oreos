#include <kernel/printk.h>
#include <kernel/sched.h>
#include <kernel/malloc/malloc.h>
#include <kernel/arch/i386/platform.h>
#include <kernel/mmu.h>

u32 _get_eflags(void);

static void *push(void *stackp, u32 v)
{
	u32* stack = stackp;
	*--stack = v;
	return stack;
}

struct cpu_state *proc_create(void *start)
{
	u8* state = kmalloc(0x1000);
	if (!state) {
		printk("failed to alloc process stack\n");
		return NULL;
	}

	void* top = &state[1024];
	u32* esp;

	top = push(top, _get_eflags());
	top = push(top, 0x08); /* cs TODO don't hard code */
	top = push(top, (u32)(uintptr_t)start); /* eip */
	top = push(top, 0); /* err_code */
	top = push(top, 0); /* vec_num */
	top = push(top, 0); /* eax */
	top = push(top, 0); /* ecx */
	top = push(top, 0); /* edx */
	top = push(top, 0); /* ebx */
	esp = top = push(top, 0); /* esp */
	top = push(top, 0); /* ebp */
	top = push(top, 0); /* esi */
	top = push(top, 0); /* edi */

	*esp = (u32) (uintptr_t) top;

	return top;
}

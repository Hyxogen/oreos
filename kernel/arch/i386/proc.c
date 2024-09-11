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

	//TODO remove magic value 3 (RPL)
	top = push(top, I386_USER_DATA_SELECTOR | 3); /* ss */
	top = push(top, 0); /* esp, TODO set */
	/* eflags: IOPL 3 (0x3000) IF: (0x0200) legacy: (0x0002) */
	top = push(top, 0x3202); /* eflags, TODO remove magic value */
	top = push(top, I386_USER_CODE_SELECTOR | 3); /* cs */
	top = push(top, (u32)(uintptr_t)start); /* eip */
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

	return top;
}

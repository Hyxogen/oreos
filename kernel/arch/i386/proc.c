#include <kernel/kernel.h>
#include <kernel/printk.h>
#include <kernel/sched.h>
#include <kernel/malloc/malloc.h>
#include <kernel/arch/i386/platform.h>
#include <kernel/arch/i386/mmu.h>
#include <kernel/libc/string.h>
#include <kernel/errno.h>
#include <kernel/irq.h>
#include <kernel/libc/assert.h>

#include <kernel/debug.h>

#define KERNEL_STACK_SIZE 0x4000

u32 _get_eflags(void);

static void *push(void *stackp, u32 v)
{
	u32* stack = stackp;
	*--stack = v;
	return stack;
}

static void proc_init(struct process *proc)
{
	proc->pid = -1;
	proc->exit_code = 0;
	proc->status = -1;
	proc->kernel_stack = NULL;
	proc->context = NULL;
	proc->next = NULL;
	proc->mm.root = NULL;
	proc->pending_signals = 0;
	memset(proc->signal_handlers, 0, sizeof(proc->signal_handlers));
	atomic_init(&proc->refcount, 1);
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

//0xc000ab84
static u32 proc_get_eflags(int ring)
{
	(void) ring;
	/* eflags: IOPL 0 (0x0000) IF: (0x0200) legacy: (0x0002) */
	//TODO remove magic values
	return 0x0202;
}

struct process *proc_create(void *start, u32 flags)
{
	struct process *proc = kmalloc(sizeof(*proc));
	if (!proc)
		return proc;
	proc_init(proc);

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

	proc->kernel_stack = mmu_mmap(NULL, KERNEL_STACK_SIZE, MMU_ADDRSPACE_KERNEL, 0);
	if (proc->kernel_stack == MMU_MAP_FAILED)
		goto err;

	void *top = (u8*) proc->kernel_stack + KERNEL_STACK_SIZE;

	u32 esp = (uintptr_t) top;
	if (ring == 3) {
		uintptr_t user_stack = MMU_KERNEL_START - KERNEL_STACK_SIZE;
		if (vma_map(&proc->mm, &user_stack, KERNEL_STACK_SIZE,
			    VMA_MAP_PROT_READ | VMA_MAP_PROT_WRITE))
			goto err;
		esp = user_stack + KERNEL_STACK_SIZE;
	}

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
		mmu_unmap(proc->kernel_stack, KERNEL_STACK_SIZE, 0);
	kfree(proc->context);
	kfree(proc);
	return NULL;
}

void proc_free(struct process *proc)
{
	mmu_unmap(proc->kernel_stack, KERNEL_STACK_SIZE, 0);
	kfree(proc);
	if (!vma_destroy(&proc->mm))
		printk("failed to properly destroy vma of pid %u\n", proc->pid);
}

struct process *proc_clone(struct process *proc, const struct cpu_state *state)
{
	struct process *cloned = kmalloc(sizeof(*proc));
	if (!cloned)
		return NULL;

	proc_init(cloned);

	cloned->kernel_stack = mmu_mmap(NULL, KERNEL_STACK_SIZE, MMU_ADDRSPACE_KERNEL, 0);
	if (cloned->kernel_stack == MMU_MAP_FAILED)
		goto err;

	u8 *top = (u8*) cloned->kernel_stack + KERNEL_STACK_SIZE;
	top -= sizeof(*cloned->context);
	memcpy(top, state, sizeof(*state));

	cloned->context = (void*) top;

	memcpy(&cloned->signal_handlers, proc->signal_handlers, sizeof(cloned->signal_handlers));

	if (vma_clone(&cloned->mm, &proc->mm))
		goto err;

	return cloned;
err:
	kfree(cloned);
	if (cloned->kernel_stack != MMU_MAP_FAILED)
		mmu_unmap(cloned->kernel_stack, KERNEL_STACK_SIZE, 0);
	return NULL;
}

void proc_prepare_switch(struct process *proc)
{
	disable_irqs();
	_tss.esp0 = (uintptr_t) proc->kernel_stack + KERNEL_STACK_SIZE;
	enable_irqs();
}

void proc_set_syscall_ret(struct cpu_state *state, size_t v)
{
	state->eax = v;
}

static int proc_push(struct cpu_state *state, const void *src, size_t n)
{
	state->esp -= n;
	int res = copy_to_user((void*)(uintptr_t)state->esp, src, n);

	if (res)
		state->esp += n;
	return res;
}

int proc_do_signal(struct process *proc, struct cpu_state *state)
{
	assert(proc->pending_signals);
	int i = __builtin_ctzll(proc->pending_signals);

	u32 mask = ~(1 << i);
	proc->pending_signals &= mask;

	if (!proc->signal_handlers[i])
		return -1;

	if (proc_push(state, state, sizeof(*state)))
		return -1;
	if (proc_push(state, &i, sizeof(i)))
		return -1; /* TODO pop previous */

	u32 dummy = 0;
	if (proc_push(state, &dummy, sizeof(dummy)))
		return -1; /* TODO pop previous */

	state->eip = (uintptr_t) proc->signal_handlers[i];
	return 0;
}

int proc_do_sigreturn(struct process *proc, struct cpu_state *state)
{
	assert(is_from_userspace(state));
	/* TODO sigaction sa_mask restore */

	u16 code, data;
	proc_get_selectors(3, &code, &data);

	state->esp += 4; /* pop dummy */
	state->esp += 4; /* pop signum */

	int res = copy_from_user(state, (void*)state->esp, sizeof(*state));
	state->esp += sizeof(*state);

	/* TODO check which other registers have to be sanitized */
	state->cs = code;
	state->ds = data;
	state->eflags.iopl = 0;
	state->eflags.ief = true;
	state->eflags._reserved1 = 1; /* legacy flag */

	return res;
}

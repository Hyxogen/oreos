#include <stdarg.h>
#include <stdbool.h>

#include <kernel/debug.h>
#include <kernel/framebuf.h>
#include <kernel/kernel.h>
#include <kernel/printk.h>
#include <kernel/ps2.h>
#include <kernel/tty.h>
#include <kernel/mmu.h>
#include <kernel/malloc/malloc.h>
#include <kernel/timer.h>
#include <kernel/platform.h>
#include <kernel/sched.h>
#include <kernel/libc/assert.h>
#include <kernel/irq.h>
#include <kernel/serial.h>

#include <kernel/acpi/acpi.h>

/* TODO REMOVE */
#include <kernel/libc/string.h>
void init_paging(void);
void init_segments(void);
void init_framebuf(struct mb2_info *info);
void start_shell(void);

static struct mb2_info *_mb2_info = NULL;
static struct term *term = NULL;
static struct acpi_table _acpi_table;

static void init_printk(void)
{
	init_framebuf(mb2_get_info());

	term = term_get_primary();
	printk_set_sink(term);
}

static void init_consoles(void)
{
	init_ps2();
}

static void read_acpi(void)
{
	const struct mb2_info *info = mb2_get_info();

	struct mb2_acpi_new *acpi =
	    (void *)mb2_find(info, MB2_TAG_TYPE_ACPI_NEW);

	if (!acpi) {
		printk("could not find ACPI tag\n");

		struct mb2_acpi_old *old =
		    (void *)mb2_find(info, MB2_TAG_TYPE_ACPI_OLD);
		if (old) {
			printk("did find legacy\n");

			acpi_read(&_acpi_table, old->rsdp);
		}
		return;
	}

	acpi_read(&_acpi_table, acpi->xsdp);
}

void dummy(void)
{
	while (1) {
		printk("dummy\n");
		__asm__ volatile("hlt");
	}
}

static void mb2_save_info(struct mb2_info *info)
{
	_mb2_info = info;
}

static void mb2_free_info(void)
{
	mmu_unmap(_mb2_info, _mb2_info->total_size, 0); // we're done with multiboot, free it
}

struct mb2_info *mb2_get_info(void)
{
	return _mb2_info;
}

extern char loop[];
extern char loop2[];
extern const unsigned char user_init[];
extern const unsigned int user_init_len;

struct process *load_elf(const void *data, size_t nbytes);

void kernel_main(struct mb2_info *info)
{
	mb2_save_info(info);

	init_early_serial();

	init_paging();
	init_segments();
	init_mmu();
	init_printk();

	read_acpi();

	init_interrupts(&_acpi_table);

	init_timer(&_acpi_table);
	init_sched();

	init_serial();

	init_consoles();

	mb2_free_info();

	printk("done!\n");

	struct process *proc = load_elf(user_init, user_init_len);
	assert(proc);
	assert(!sched_schedule(proc));
	proc_release(proc);

	/*uintptr_t dummy1_start = 0x400000;
	struct process *dummy = proc_create((void*)dummy1_start, PROC_FLAG_RING3);

	assert(dummy);
	assert(!sched_schedule(dummy));

	assert(!vma_map(&dummy->mm, &dummy1_start, MMU_PAGESIZE, VMA_MAP_PROT_READ | VMA_MAP_FIXED_NOREPLACE));

	assert(dummy->mm.root->left);

	assert(!vma_map_now_one(dummy->mm.root->left, dummy1_start));
	void *tmp = mmu_map_pages(NULL, dummy->mm.root->left->pages[0], 1, MMU_ADDRSPACE_KERNEL, 0);
	assert(tmp != MMU_MAP_FAILED);

	memcpy((void*)tmp, loop, MMU_PAGESIZE);

	mmu_unmap(tmp, MMU_PAGESIZE, 0);

	proc_release(dummy);*/

	sched_start();
}

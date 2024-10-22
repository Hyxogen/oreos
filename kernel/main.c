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

	init_vma();

	printk("done!\n");

	struct mm dummy1_mm, dummy2_mm;
	dummy1_mm.root = dummy2_mm.root = NULL;
	uintptr_t dummy1_start = 0, dummy2_start = 0;
	assert(!vma_map(&dummy1_mm, &dummy1_start, MMU_PAGESIZE, (uintptr_t)loop));
	assert(!vma_map(&dummy2_mm, &dummy2_start, MMU_PAGESIZE, (uintptr_t)loop2));
	assert(!vma_map_now(dummy1_mm.root));
	memcpy((void*)dummy1_start, loop, MMU_PAGESIZE);
	assert(!vma_map_now(dummy2_mm.root));
	memcpy((void*)dummy2_start, loop2, MMU_PAGESIZE);

	struct process *dummy = proc_create((void*)dummy1_start, PROC_FLAG_RING3);

	assert(dummy);
	struct process *dummy2 = proc_create((void*)dummy2_start, PROC_FLAG_RING3);
	assert(dummy2);
	assert(!sched_proc(dummy));
	assert(!sched_proc(dummy2));

	dummy->mm.root = dummy1_mm.root;
	dummy2->mm.root = dummy2_mm.root;

	BOCHS_BREAK;
	sched_start();
}

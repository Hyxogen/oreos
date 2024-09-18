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
	mmu_unmap(_mb2_info, _mb2_info->total_size); // we're done with multiboot, free it
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

	init_consoles();

	mb2_free_info();

	printk("done!\n");

	void *temp = mmu_map(NULL, (uintptr_t)loop - 0xC0000000, MMU_PAGESIZE,
			     MMU_ADDRSPACE_USER, 0);
	assert(temp != MMU_MAP_FAILED);
	void *temp2 = mmu_map(NULL, (uintptr_t)loop2 - 0xC0000000, MMU_PAGESIZE,
			     MMU_ADDRSPACE_USER, 0);
	assert(temp2 != MMU_MAP_FAILED);

	struct process *dummy = proc_create(temp, PROC_FLAG_RING3);
	assert(dummy);
	struct process *dummy2 = proc_create(temp2, PROC_FLAG_RING3);
	assert(dummy2);
	assert(!sched_proc(dummy));
	assert(!sched_proc(dummy2));

	sched_start();
}

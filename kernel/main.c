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

#include <kernel/acpi/acpi.h>

void init_paging(void);
void init_segments(void);
void init_framebuf(struct mb2_info *info);
void start_shell(void);
void init_irq_handler(struct acpi_table *table);

static struct mb2_info *_mb2_info = NULL;
static struct term *term = NULL;
static struct acpi_table _acpi_table;

static void init_consoles(void)
{
	init_framebuf(mb2_get_info());

	term = term_get_primary();
	printk_set_sink(term);
	init_ps2();
}


static void read_acpi(void)
{
	const struct mb2_info *info = mb2_get_info();

	struct mb2_acpi_new *acpi = (void*) mb2_find(info, MB2_TAG_TYPE_ACPI_NEW);

	if (!acpi) {
		printk("could not find ACPI tag\n");

		struct mb2_acpi_old *old = (void*) mb2_find(info, MB2_TAG_TYPE_ACPI_OLD);
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

struct mb2_info *mb2_get_info(void)
{
	return _mb2_info;
}

void kernel_main(struct mb2_info *info)
{
	mb2_save_info(info);

	init_paging();
	init_segments();
	init_mmu();
	init_consoles();
	read_acpi();
	init_irq_handler(&_acpi_table);
	init_timer(&_acpi_table);
	//init_printk(); TODO
	
	printk("a\n");
	//timer_sleep(2000);
	printk("b\n");

	mmu_unmap(info, info->total_size); // we're done with multiboot, free it

	printk("done!\n");
	init_sched();

	struct cpu_state *state = proc_create(start_shell);
	assert(state);
	struct cpu_state *dummystate = proc_create(dummy);
	assert(dummystate);

	assert(!sched_proc(state));
	assert(!sched_proc(dummystate));

	sched_start();
}

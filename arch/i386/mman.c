#include <kernel/mman.h>
#include <kernel/printk.h>
#include "multiboot2.h"

static void mman_dump_entry(const struct multiboot_mmap_entry *entry)
{
	printk("[0x%09llx, 0x%09llx) -> ", entry->base_addr, entry->base_addr + entry->length);

	switch (entry->type) {
	case MULTIBOOT_MMAP_TYPE_AVAIL:
		printk("available");
		break;
	case MULTIBOOT_MMAP_TYPE_ACPI_RECLAIMABLE:
		printk("ACPI reclaimable");
		break;
	case MULTIBOOT_MMAP_TYPE_RESERVED_HIBER:
		printk("hibernation reserved");
		break;
	case MULTIBOOT_MMAP_TYPE_BADRAM:
		printk("bad ram");
		break;
	default:
		printk("reserved");
	}
	printk(" (%lu)\n", entry->type);
}

static void mman_dump_entries(const struct multiboot_mmap *map)
{
	size_t size = map->base.size - 4 * sizeof(u32);
	size_t avail = 0;

	for (size_t i = 0; i < size; i += map->entry_size) {
		struct multiboot_mmap_entry *entry = (struct multiboot_mmap_entry*) &map->entries[i];
		if (entry->type == MULTIBOOT_MMAP_TYPE_AVAIL)
			avail += entry->length;
		mman_dump_entry(entry);
		//printk("addr: %llx length: %llx\n", entry->base_addr, entry->length);
	}
	printk("total avail: 0x%08zx\n", avail);
}

void mman_dump(void)
{
	const struct multiboot_tag_base *tag =
	    (const struct multiboot_tag_base *)_multiboot_data->tags;

	for (; tag->type != MULTIBOOT_TAG_TYPE_END;
	     tag = MULTIBOOT_NEXT_TAG(tag)) {
		if (tag->type == MUTLIBOOT_TAG_TYPE_MMAP) {
			printk("found mmap data\n");

			mman_dump_entries((const struct multiboot_mmap*) tag);
		}
	}
}

void mman_init(void)
{

}

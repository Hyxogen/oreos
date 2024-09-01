#include <kernel/acpi.h>
#include <kernel/align.h>
#include <kernel/malloc/malloc.h>
#include <kernel/mmu.h>
#include <kernel/printk.h>
#include <lib/string.h>

bool acpi_validate(const void *data, size_t len)
{
	const unsigned char *data_c = data;
	u8 sum = 0;

	while (len--) {
		sum += *data_c++;
	}
	return sum == 0;
}

static void dump_sdt(const struct sdt_hdr *hdr)
{
	/* TODO be more consisted with *_id or *id */
	printk("Signature=\"%.4s\" ", hdr->signature);
	printk("OEMID=\"%.6s\" ", hdr->oemid);
	printk("OEMTableID=\"%.6s\"\n", hdr->oem_tableid);

	printk("Revision=%hhu ", hdr->rev);
	printk("CreatorID=0x%lx ", hdr->creator_id);
	printk("CreatorRevision=0x%lx\n", hdr->creator_rev);
}

static struct sdt_hdr *copy_sdt_entry(uintptr_t ptr)
{
	struct sdt_hdr *hdr =
	    mmu_map(NULL, ptr, MMU_PAGESIZE, MMU_ADDRSPACE_KERNEL, 0);
	if (!hdr)
		return NULL;

	void *res = NULL;

	if (hdr->len < MMU_PAGESIZE) {

		if (acpi_validate(hdr, hdr->len)) {
			dump_sdt(hdr);
			res = kmalloc(hdr->len);
			if (res)
				memcpy(res, hdr, hdr->len);
			else
				printk("failed to allocate %lu bytes to copy sdt\n",
				       hdr->len);
		} else {
			printk("sdt checksum failed\n");
		}
	} else {
		printk("rsdt entry too large\n");
	}

	mmu_unmap(hdr, MMU_PAGESIZE);

	return res;
}

static bool read_rsdt_entries(struct acpi_table *table, struct rsdt *rsdt)
{
	size_t count =
	    (rsdt->hdr.len - sizeof(rsdt->hdr)) / sizeof(rsdt->entry_ptrs[0]);
	table->entries = kmalloc(count * sizeof(table->entries[0]));

	if (!table->entries) {
		printk("failed to allocate %zu ACPI table entries\n", count);
		return false;
	}

	printk("found %zu ACPI table entries\n", count);
	for (; table->count < count; table->count++) {
		table->entries[table->count] =
		    copy_sdt_entry(rsdt->entry_ptrs[table->count]);

		if (!table->entries[table->count])
			return false;
	}
	return true;
}

bool read_rsdp(struct acpi_table *table, struct rsdp *root)
{
	if (!acpi_validate(root, sizeof(*root))) {
		printk("RSDP checksum failed\n");
		return false;
	}

	bool success = true;
	struct rsdt *rsdt = mmu_map(NULL, root->rsdt_addr, MMU_PAGESIZE,
				    MMU_ADDRSPACE_KERNEL, 0);
	if (rsdt == MMU_MAP_FAILED) {
		printk("failed to map RSDT\n");
		return false;
	}

	if (rsdt->hdr.len > MMU_PAGESIZE) {
		// TODO support arbitrary size RSDT
		printk("RSDT too large\n");
		success = false;
	} else {
		dump_sdt(&rsdt->hdr);
		success = read_rsdt_entries(table, rsdt);

		if (!success)
			acpi_free(table);
	}

	mmu_unmap(rsdt, MMU_PAGESIZE);
	return success;
}

static void acpi_init_table(struct acpi_table *table)
{
	table->version = -1;
	table->count = 0;
	table->entries = 0;
}

void acpi_free(struct acpi_table *table)
{
	for (size_t i = 0; i < table->count; i++) {
		kfree(table->entries[i]);
	}
	kfree(table->entries);

	table->count = 0;
	table->entries = NULL;
}

bool acpi_read(struct acpi_table *table, void *rsdpp)
{
	struct rsdp *rsdp = rsdpp;

	acpi_init_table(table);

	printk("detected ACPI, rev=%hhx signature=\"%.8s\" oemid=\"%.6s\"\n",
	       rsdp->rev, rsdp->signature, rsdp->oemid);

	switch (rsdp->rev) {
	case 0:
		table->version = ACPI_LEGACY;
		return read_rsdp(table, rsdp);
	default:
		printk("unsupported ACPI revision %hhu\n", rsdp->rev);
		return false;
	}
}

struct sdt_hdr *acpi_find(const struct acpi_table *table, const char *signature)
{
	for (size_t i = 0; i < table->count; i++) {
		struct sdt_hdr *entry = table->entries[i];
		if (!strncmp(entry->signature, signature, ACPI_SDT_SIG_LEN)) {
			return entry;
		}
	}
	return NULL;
}

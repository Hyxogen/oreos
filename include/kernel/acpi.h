#ifndef __KERNEL_ACPI_H
#define __KERNEL_ACPI_H

#include <stdbool.h>
#include <stddef.h>
#include <kernel/types.h>

#define ACPI_SDT_SIG_LEN 4

#define ACPI_LEGACY 1
#define ACPI_NORMAL 2

#define MADT_TYPE_LAPIC 0
#define MADT_TYPE_IOAPIC 1
#define MADT_TYPE_IOAPIC_SRC_OVERRIDE 2
#define MADT_TYPE_IOAPIC_NMI_SRC 3
#define MADT_TYPE_LAPIC_NMI 4
#define MADT_TYPE_LAPIC_ADDR_OVERRIDE 5
#define MADT_TYPE_2XLAPIC 9

#define ACPI_TAG_APIC "APIC"

struct rsdp {
	char signature[8];
	u8 checksum;
	char oemid[6];
	u8 rev;
	u32 rsdt_addr;
} __attribute__ ((packed));

struct xsdp {
	struct rsdp base;

	u32 len;
	u64 xsdt_addr;
	u8 ext_checksum;
	u8 reserved[3];
} __attribute__ ((packed));

// TODO check if packed attribute is needed
struct sdt_hdr {
	char signature[ACPI_SDT_SIG_LEN];
	u32 len;
	u8 rev;
	u8 checksum;
	char oemid[6];
	char oem_tableid[8];
	u32 oem_rev;
	u32 creator_id;
	u32 creator_rev;
};

struct rsdt {
	struct sdt_hdr hdr;
	u32 entry_ptrs[]; /* (hdr.len - sizeof(hdr)) / 4 */
};

struct xsdt {
	struct sdt_hdr hdr;
	u64 entry_ptrs[] __attribute__((aligned(4))); /* (hdr.len - sizeof(hdr)) / 8 */
};

struct madt {
	struct sdt_hdr hdr;

	u32 lapic_addr;
	u32 flags;
};

struct acpi_table {
	int version;

	size_t count;
	struct sdt_hdr **entries;
};

struct madt_record {
	u8 type;
	u8 len;
};

struct madt_lapic {
	u8 apic_cpu_id;
	u8 apic_id;
	u32 flags;
} __attribute__((packed));

bool acpi_validate(const void *data, size_t len);
/* this validates the rsdp/xsdp and maps the sdt */
bool acpi_read(struct acpi_table *table, void *rsdpp);
void acpi_free(struct acpi_table *table);
struct sdt_hdr *acpi_find(const struct acpi_table *table, const char *signature);

#endif

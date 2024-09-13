#include <kernel/printk.h>
#include <kernel/mmu.h>
#include <kernel/arch/i386/apic.h>
#include <kernel/arch/i386/io.h>
#include <kernel/libc/assert.h>
#include <kernel/malloc/malloc.h>
#include <kernel/libc/string.h>

#include <kernel/debug.h>

#define PIC_MASTER_CMD_PORT 0x20
#define PIC_MASTER_DATA_PORT 0x21
#define PIC_SLAVE_CMD_PORT 0xA0
#define PIC_SLAVE_DATA_PORT 0xA1

#define PIC_ICW1_START_INIT 0x10
#define PIC_ICW1_ICW4 0x01
#define PIC_ICW4_8086_MODE 0x01

#define LAPIC_SPURRIOUS_IVR 0xF0

/* used for sanity checks */
#define APIC_MAX_IOAPICS 128
#define APIC_MAX_OVERRIDES 128
#define APIC_MAX_LAPICS 128

struct ioapic {
	void *addr;
	u32 gsi_base;

	u8 id;
	u8 nredirects;
};

struct lapic {
	u8 cpu_id;
	u8 id;
	u32 flags;
};

struct ioapic_override {
	u32 gsi;
	u16 flags;
	u8 bus_src;
	u8 irq_src;
};

struct apic {
	void *lapic_addr;
	u32 flags;

	u8 nioapic;
	struct ioapic *ioapics;

	u8 noverride;
	struct ioapic_override *overrides;

	u8 nlapic;
	struct lapic *lapics;
};

static struct apic _apic;

static void outb_and_wait(u16 port, u8 b)
{
	outb(port, b);
	io_wait();
}

void ioapic_write(void *ioapicaddr, u32 reg, u32 val)
{
	volatile u32 *ioapic = (volatile u32*)ioapicaddr;
	ioapic[0] = reg & 0xff;
	ioapic[4] = val;
}

u32 ioapic_read(void *ioapicaddr, u32 reg)
{
	volatile u32 *ioapic = (volatile u32*)ioapicaddr;
	ioapic[0] = reg & 0xff;
	return ioapic[4];
}

/* remaps the PIC IRQ vector numbers to start at the specified offsets */
static void pic_remap(u8 master_offset, u8 slave_offset)
{
	u8 m1, m2;

	/* save masks */
	m1 = inb(PIC_MASTER_DATA_PORT);
	m2 = inb(PIC_SLAVE_DATA_PORT);

	/* ICW1: start init sequence, and indicate that we'll send an ICW4 */
	outb_and_wait(PIC_MASTER_CMD_PORT, PIC_ICW1_START_INIT | PIC_ICW1_ICW4);
	outb_and_wait(PIC_SLAVE_CMD_PORT, PIC_ICW1_START_INIT | PIC_ICW1_ICW4);

	/* set the interrupt vector offset */
	outb_and_wait(PIC_MASTER_DATA_PORT, master_offset);
	outb_and_wait(PIC_SLAVE_DATA_PORT, slave_offset);

	/* ICW3: inform master that there is a slave PIC at IRQ2 (100b) */
	outb_and_wait(PIC_MASTER_DATA_PORT, 4);
	/* inform slave that its identity is PIC number 2 */
	outb_and_wait(PIC_SLAVE_DATA_PORT, 2);


	/* ICW4: set PICs to 8086 mode */
	outb_and_wait(PIC_MASTER_DATA_PORT, PIC_ICW4_8086_MODE);
	outb_and_wait(PIC_SLAVE_DATA_PORT, PIC_ICW4_8086_MODE);

	/* restore masks */
	outb(PIC_MASTER_DATA_PORT, m1);
	outb(PIC_SLAVE_DATA_PORT, m2);
}

static void pic_disable(void)
{
	outb(PIC_MASTER_DATA_PORT, 0xff);
	outb(PIC_SLAVE_DATA_PORT, 0xff);
}

static volatile u32* lapic_get_reg(void *base_addr, u32 reg)
{
	return (volatile u32 *)((u8 *)base_addr + reg);
}

static void lapic_write(void *base_addr, u32 reg, u32 val)
{
	volatile u32 *p = lapic_get_reg(base_addr, reg);
	*p = val;
}

static u32 lapic_read(void *base_addr, u32 reg)
{
	return *lapic_get_reg(base_addr, reg);
}

static void linit_apic(void *base_addr)
{
	u32 val = lapic_read(base_addr, LAPIC_SPURRIOUS_IVR);

	val |= LAPIC_SPURRIOUS_IRQN; /* set IRQ number */
	val |= 0x100; /* enable */
	lapic_write(base_addr, LAPIC_SPURRIOUS_IVR, val);
}

static bool apic_read_ioapics(struct apic *dest, const struct madt *madt)
{
	size_t count = madt_count(madt, MADT_TYPE_IOAPIC);

	if (count >= APIC_MAX_IOAPICS)
		return false;

	dest->ioapics = kmalloc(sizeof(*dest->ioapics) * count);
	if (!dest->ioapics)
		return false;

	dest->nioapic = 0;

	u8 i = 0;
	struct madt_record *record = madt_first(madt);
	while (record) {
		if (record->type == MADT_TYPE_IOAPIC) {
			struct madt_ioapic *ioapic_rec = (struct madt_ioapic*) record;
			struct ioapic *ioapic = &dest->ioapics[i];

			ioapic->addr =
			    mmu_map(NULL, ioapic_rec->ioapic_addr, MMU_PAGESIZE,
				    MMU_ADDRSPACE_KERNEL, 0);
			if (ioapic->addr == MMU_MAP_FAILED)
				goto err;

			ioapic->id = ioapic_rec->ioapic_id;
			ioapic->gsi_base = ioapic_rec->global_system_interrupt_base;
			ioapic->nredirects = (ioapic_read(ioapic->addr, IOAPIC_REG_VERSION) & 0xff0000) >> 16;

			dest->nioapic += 1;
			i += 1;
		}
		record = madt_next(madt, record);
	}

	return true;
err:
	for (u8 i = 0; i < dest->nioapic; i++) {
		mmu_unmap(dest->ioapics[i].addr, MMU_PAGESIZE);
	}
	kfree(dest->ioapics);
	dest->nioapic = 0;
	return false;
}

static bool apic_read_overrides(struct apic *dest, struct madt *madt)
{
	size_t count = madt_count(madt, MADT_TYPE_IOAPIC_SRC_OVERRIDE);
	if (count > APIC_MAX_OVERRIDES) {
		printk("IOAPIC: too many src overrides (%zu)\n", count);
		return false;
	}

	dest->overrides = kmalloc(sizeof(*dest->overrides) * count);
	if (!dest->overrides) {
		printk("failed to alloc overrides\n");
		return false;
	}

	unsigned i = 0;
	struct madt_record *record = madt_first(madt);
	while (record) {
		if (record->type == MADT_TYPE_IOAPIC_SRC_OVERRIDE) {
			struct madt_ioapic_override *override_rec = (struct madt_ioapic_override*) record;
			struct ioapic_override *override = &dest->overrides[i];

			override->bus_src = override_rec->bus_src;
			override->irq_src = override_rec->irq_src;
			override->flags = override_rec->flags;
			override->gsi = override_rec->global_system_interrupt;

			dest->noverride += 1;
			i += 1;
		}
		record = madt_next(madt, record);
	}
	return true;
}

static bool apic_read_lapics(struct apic *dest, struct madt *madt)
{
	size_t count = madt_count(madt, MADT_TYPE_LAPIC);
	if (count > APIC_MAX_LAPICS) {
		printk("APIC: too many Local APICs, found %zu, will only use %u\n", count, APIC_MAX_LAPICS);
		count = APIC_MAX_LAPICS;
	}

	dest->lapics = kmalloc(sizeof(*dest->lapics) * count);
	if (!dest->lapics) {
		printk("APIC: failed to allocate %zu bytes for storing LAPICs\n", count * sizeof(*dest->lapics));
		return false;
	}

	unsigned i = 0;
	struct madt_record *record = madt_first(madt);
	while (record) {
		if (record->type == MADT_TYPE_LAPIC) {
			struct madt_lapic *lapic_rec = (struct madt_lapic*) record;
			struct lapic *lapic = &dest->lapics[i];

			lapic->cpu_id = lapic_rec->lapic_cpu_id;
			lapic->id = lapic_rec->lapic_id;
			lapic->flags = lapic_rec->flags;

			dest->nlapic += 1;
			i += 1;
		}
		record = madt_next(madt, record);
	}

	return true;
}

static bool apic_read(struct apic *dest, struct madt *madt)
{
	memset(dest, 0, sizeof(*dest));

	dest->lapic_addr = mmu_map(NULL, madt->lapic_addr, MMU_PAGESIZE, MMU_ADDRSPACE_KERNEL, 0);
	if (!dest->lapic_addr)
		return false;
	dest->flags = madt->flags;

	if (!apic_read_ioapics(dest, madt))
		printk("failed to read ioapics\n");

	if (!apic_read_overrides(dest, madt))
		printk("failed to read ioapic overrides\n");

	if (!apic_read_lapics(dest, madt))
		printk("failed to read lapics\n");

	return true;
}

void init_apic(struct madt *madt)
{
	//TODO keep track somewhere of these offsets
	pic_remap(0x20, 0x28);
	printk("remapped PIC offsets to 0x20 and 0x28\n");
	pic_disable();
	printk("disabled PICs\n");

	assert(apic_read(&_apic, madt));
	
	linit_apic(_apic.lapic_addr);
}

void lapic_eoi(void)
{
	assert(_apic.lapic_addr);
	lapic_write(_apic.lapic_addr, 0xB0, 0);
}

static struct ioapic *ioapic_find(u8 irq_src)
{
	for (u8 i = 0; i < _apic.nioapic; i++) {
		struct ioapic *ioapic = &_apic.ioapics[i];
		if (irq_src >= ioapic->gsi_base &&
		    irq_src < ioapic->gsi_base + ioapic->nredirects)
			return ioapic;
	}
	return NULL;
}

bool ioapic_set_redir(u8 irq_src, u64 val)
{
	for (u8 i = 0; i < _apic.noverride; i++) {
		if (_apic.overrides[i].irq_src == irq_src) {
			irq_src = _apic.overrides[i].gsi;
			break;
		}
	}

	struct ioapic *ioapic = ioapic_find(irq_src);
	if (!ioapic)
		return false;

	void *ioapicaddr = ioapic->addr;
	ioapic_write(ioapicaddr, IOAPIC_REG_REDIR(irq_src), val & 0xffffffff);
	ioapic_write(ioapicaddr, IOAPIC_REG_REDIR(irq_src) + 1, val >> 32);
	return true;

}

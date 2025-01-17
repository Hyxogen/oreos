#ifndef __KERNEL_APIC_H
#define __KERNEL_APIC_H

#include <kernel/acpi/acpi.h>

#define IOAPIC_PRIO_NORMAL 0x0000 
#define IOAPIC_PRIO_LOW 0x0100 
#define IOAPIC_PRIO_SYSTEM 0x0200 
#define IOAPIC_PRIO_NMI 0x0400 
#define IOAPIC_PRIO_INIT 0x0500 
#define IOAPIC_PRIO_EXTERNAL 0x0700 

#define IOAPIC_DEST_PHYSICAL 0x0000
#define IOAPIC_DEST_LOGICAL 0x0800

#define IOAPIC_POLARITY_HIGHACTIVE 0x0000
#define IOAPIC_POLARITY_LOWACTIVE 0x2000

#define IOAPIC_TRIGGER_EDGE 0x0000
#define IOAPIC_TRIGGER_LEVEL 0x8000

#define IOAPIC_MASKED 0x10000

#define IOAPIC_PHYSICAL_ID(id) ((u64) (id) << 56)

#define IOAPIC_REG_REDIR(i) (0x10 + 2 * (i))

#define IOAPIC_VECTOR(i) ((u8)i)

#define IOAPIC_REG_VERSION 0x01

#define IOAPIC_REDIR(prio, dest, polarity, trigger, destid, vec) (prio | dest | polarity | trigger | destid | vec)

#define LAPIC_SPURRIOUS_IRQN 0xff

void init_apic(struct madt *madt);
void lapic_eoi(void);

bool ioapic_set_redir(u8 idx, u64 val);

#endif

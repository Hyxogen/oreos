#ifndef __KERNEL_APIC_H
#define __KERNEL_APIC_H

#include <kernel/acpi.h>

void apic_init(struct madt *madt);

#endif

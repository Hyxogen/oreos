#ifndef __KERNEL_ARCH_I386_PLATFORM_H
#define __KERNEL_ARCH_I386_PLATFORM_H

#include <kernel/types.h>
#include <kernel/platform.h>

#define I386_GDT_KERNEL_CODE_IDX 1
#define I386_GDT_KERNEL_DATA_IDX 2

#define I386_GDT_USER_CODE_IDX 3
#define I386_GDT_USER_DATA_IDX 4

#define I386_SELECTOR_IDX_SHIFT 3
#define I386_IDX_TO_SELECTOR(i) ((i) << I386_SELECTOR_IDX_SHIFT)

#define I386_KERNEL_CODE_SELECTOR I386_IDX_TO_SELECTOR(I386_GDT_KERNEL_CODE_IDX)
#define I386_KERNEL_DATA_SELECTOR I386_IDX_TO_SELECTOR(I386_GDT_KERNEL_DATA_IDX)
#define I386_USER_CODE_SELECTOR I386_IDX_TO_SELECTOR(I386_GDT_USER_CODE_IDX)
#define I386_USER_DATA_SELECTOR I386_IDX_TO_SELECTOR(I386_GDT_USER_DATA_IDX)

struct cpu_state {
	u32 edi;
	u32 esi;
	u32 ebp;
	u32 esp;
	u32 ebx;
	u32 edx;
	u32 ecx;
	u32 eax;

	u32 vec_num;
	u32 err_code;

	u32 eip;
	u16 cs;
	u16 _reserved;

	u32 eflags;
};


#endif
#ifndef __KERNEL_ARCH_I386_PLATFORM_H
#define __KERNEL_ARCH_I386_PLATFORM_H

#include <kernel/types.h>
#include <kernel/platform.h>

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

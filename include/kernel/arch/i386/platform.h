#ifndef __KERNEL_ARCH_I386_PLATFORM_H
#define __KERNEL_ARCH_I386_PLATFORM_H

#include <kernel/types.h>
#include <kernel/platform.h>

#define I386_GDT_KERNEL_CODE_IDX 1
#define I386_GDT_KERNEL_DATA_IDX 2

#define I386_GDT_USER_CODE_IDX 3
#define I386_GDT_USER_DATA_IDX 4

#define I386_GDT_TSS_IDX 5

#define I386_SELECTOR_IDX_SHIFT 3
#define I386_IDX_TO_SELECTOR(i) ((i) << I386_SELECTOR_IDX_SHIFT)

#define I386_KERNEL_CODE_SELECTOR I386_IDX_TO_SELECTOR(I386_GDT_KERNEL_CODE_IDX)
#define I386_KERNEL_DATA_SELECTOR I386_IDX_TO_SELECTOR(I386_GDT_KERNEL_DATA_IDX)
#define I386_USER_CODE_SELECTOR I386_IDX_TO_SELECTOR(I386_GDT_USER_CODE_IDX)
#define I386_USER_DATA_SELECTOR I386_IDX_TO_SELECTOR(I386_GDT_USER_DATA_IDX)
#define I386_TSS_SELECTOR I386_IDX_TO_SELECTOR(I386_GDT_TSS_IDX)

#define IOPL_USER 3

struct gdtr {
	u16 limit;
	u32 base;
} __attribute__((packed));

struct tss {
	u32 prev_tss;
	u32 esp0;
	u32 ss0;
	u32 esp1;
	u32 ss1;
	u32 esp2;
	u32 ss2;
	u32 cr3;
	u32 eip;
	u32 eflags;
	u32 eax;
	u32 ecx;
	u32 edx;
	u32 ebx;
	u32 esp;
	u32 ebp;
	u32 esi;
	u32 edi;
	u32 es;
	u32 cs;
	u32 ss;
	u32 ds;
	u32 fs;
	u32 gs;
	u32 ldt;
	u16 trap;
	u16 iomap_base;

} __attribute__((packed));

struct eflags {
	union {
		u32 val;
		struct {
			bool cf : 1;
			u32 _reserved1 : 1;
			bool pf : 1;
			u32 _reserved2 : 1;
			bool af : 1;
			u32 _reserved3 : 1;
			bool zf : 1;
			bool sf : 1;
			bool tf : 1;
			bool ief : 1;
			bool df : 1;
			bool of : 1;
			u32 iopl : 2;
			bool nt : 1;
			u32 _reserved4 : 1;
			bool rf : 1;
			bool vm : 1;
			bool ac : 1;
			bool vif : 1;
			bool vip : 1;
			bool id : 1;
			u32 _reserved : 10;
		} __attribute__((packed));
	};
} __attribute__((packed));

_Static_assert(sizeof(struct eflags) == 4, "basic assumption");

struct cpu_state {
	u32 ds;
	u32 edi;
	u32 esi;
	u32 ebp;
	u32 old_esp;
	u32 ebx;
	u32 edx;
	u32 ecx;
	u32 eax;

	u32 vec_num;
	u32 err_code;

	u32 eip;
	u16 cs;
	u16 _reserved1;

	struct eflags eflags;

	/* only available on privilege transition */
	u32 esp;
	u16 ss;
	u16 _reserved2;
} __attribute__((packed));

extern struct tss _tss;

void _reload_segments(u16 code, u16 data);

#endif

#include <kernel/arch/i386/platform.h>
#include <kernel/types.h>
#include <stdbool.h>
#include <stddef.h>

#define GDT_ACCESS_TYPE_SYSTEM 0
#define GDT_ACCESS_TYPE_CODEDATA 1

#define GDT_ACCESS_DC_GROW_UP 0
#define GDT_ACCESS_DC_GROW_DOWN 1

#define GDT_FLAG_SIZE_16BIT 0
#define GDT_FLAG_SIZE_32BIT 1

#define GDT_ACCESS_PRESENT(x) (0x0080 * (x))
#define GDT_ACCESS_TYPE(x) (0x0010 * (x))
#define GDT_ACCESS_EXEC(x) (0x0008 * (x))
#define GDT_ACCESS_RING(x) (((x) & 0x0003) << 0x0005)
#define GDT_ACCESS_DC(x) (0x0004 * (x))
#define GDT_ACCESS_RW(x) (0x0002 * (x))
#define GDT_ACCESS_ACCESSED(x) (0x0001 * (x))
#define GDT_FLAG_GRAN(x) (0x8000 * (x))
#define GDT_FLAG_SIZE(x) (0x4000 * (x)) // 0 for 16 bit 1 for 32 bit
#define GDT_FLAG_LONG(x) (0x2000 * (x))

#define GDT_DATA(ring)                                                       \
	(GDT_ACCESS_PRESENT(1) | GDT_ACCESS_TYPE(GDT_ACCESS_TYPE_CODEDATA) | \
	 GDT_ACCESS_EXEC(0) | GDT_ACCESS_RING(ring) |                        \
	 GDT_ACCESS_DC(GDT_ACCESS_DC_GROW_UP) | GDT_ACCESS_RW(1) |           \
	 GDT_ACCESS_ACCESSED(1) | GDT_FLAG_GRAN(1) |                         \
	 GDT_FLAG_SIZE(GDT_FLAG_SIZE_32BIT) | GDT_FLAG_LONG(0))

#define GDT_STACK(ring)                                                      \
	(GDT_ACCESS_PRESENT(1) | GDT_ACCESS_TYPE(GDT_ACCESS_TYPE_CODEDATA) | \
	 GDT_ACCESS_EXEC(0) | GDT_ACCESS_RING(ring) |                        \
	 GDT_ACCESS_DC(GDT_ACCESS_DC_GROW_DOWN) | GDT_ACCESS_RW(1) |         \
	 GDT_ACCESS_ACCESSED(1) | GDT_FLAG_GRAN(1) |                         \
	 GDT_FLAG_SIZE(GDT_FLAG_SIZE_32BIT) | GDT_FLAG_LONG(0))

#define GDT_CODE(ring)                                                       \
	(GDT_ACCESS_PRESENT(1) | GDT_ACCESS_TYPE(GDT_ACCESS_TYPE_CODEDATA) | \
	 GDT_ACCESS_EXEC(1) | GDT_ACCESS_RING(ring) |                        \
	 GDT_ACCESS_DC(GDT_ACCESS_DC_GROW_UP) | GDT_ACCESS_RW(1) |           \
	 GDT_ACCESS_ACCESSED(1) | GDT_FLAG_GRAN(1) |                         \
	 GDT_FLAG_SIZE(GDT_FLAG_SIZE_32BIT) | GDT_FLAG_LONG(0))

struct gdtr {
	u16 limit;
	u32 base;
} __attribute__((packed));

static u64 _gdt[16];

static u64 gdt_encode(u32 base, u32 limit, u16 flags)
{
	u64 res;

	res = base & 0xff000000;
	res |= (flags << 8) & 0x00f0ff00;
	res |= (base >> 16) & 0x000000ff;
	res |= limit & 0x000f0000;

	res <<= 32;

	res |= base << 16;
	res |= limit & 0x0000ffff;

	return res;
}

static void lgdt(u32 base, u16 limit)
{
	struct gdtr gdtr;

	gdtr.limit = limit;
	gdtr.base = base;
	// TODO make sure interrupts are disabled
	__asm__ volatile("lgdt %0" : : "m"(gdtr) : "memory");
}

void _reload_segments(u16 code, u16 data);

static void load_gdt(u64 *gdt, size_t entries)
{
	lgdt((u32)(uintptr_t)gdt, (entries * sizeof(*gdt)) - 1);
}

void init_segments(void)
{
	_gdt[0] = gdt_encode(0, 0, 0);
	_gdt[I386_GDT_KERNEL_CODE_IDX] = gdt_encode(0, 0x000fffff, GDT_CODE(0));
	_gdt[I386_GDT_KERNEL_DATA_IDX] = gdt_encode(0, 0x000fffff, GDT_DATA(0));
	_gdt[I386_GDT_USER_CODE_IDX] = gdt_encode(0, 0x000fffff, GDT_CODE(3));
	_gdt[I386_GDT_USER_DATA_IDX] = gdt_encode(0, 0x000fffff, GDT_DATA(3));
	// TODO TSS segment

	load_gdt(_gdt, 5);
	_reload_segments(I386_KERNEL_CODE_SELECTOR, I386_KERNEL_DATA_SELECTOR);
}

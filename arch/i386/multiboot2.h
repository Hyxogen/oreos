#ifndef OREOS_MULTIBOOT2
#define OREOS_MULTIBOOT2

#include <kernel/types.h>

#define MULTIBOOT_TAG_TYPE_END 0
#define MUTLIBOOT_TAG_TYPE_MMAP 6
#define MULTIBOOT_TAG_TYPE_FRAMEBUFFER 8

#define MULTIBOOT_MMAP_TYPE_AVAIL 1
#define MULTIBOOT_MMAP_TYPE_ACPI_RECLAIMABLE 3
#define MULTIBOOT_MMAP_TYPE_RESERVED_HIBER 4
#define MULTIBOOT_MMAP_TYPE_BADRAM 5

#define FRAMEBUFFER_TYPE_PALETTE 0
#define FRAMEBUFFER_TYPE_DIRECT 1
#define FRAMEBUFFER_TYPE_EGA	2

struct multiboot_info {
	u32 total_size;
	u32 reserved;
	u8 tags[] __attribute__((aligned(8)));
};

struct multiboot_tag_base {
	u32 type;
	u32 size;
};

struct multiboot_color_desc {
	u8 red;
	u8 green;
	u8 blue;
};

struct multiboot_palette {
	u32 num_colors;
	struct multiboot_color_desc palette[];
};

struct multiboot_direct_color {
	u8 field_pos;
	u8 mask_size;
};

struct multiboot_direct_rgb {
	union {
		struct {
			struct multiboot_direct_color red;
			struct multiboot_direct_color green;
			struct multiboot_direct_color blue;
		};
		struct multiboot_direct_color colors[3];
	};
};

struct multiboot_mmap_entry {
	u64 base_addr;
	u64 length;
	u32 type;
	u32 reserved;
};

struct multiboot_mmap {
	struct multiboot_tag_base base;
	u32 entry_size;
	u32 entry_version;
	u8 entries[];
};

struct multiboot_framebuffer_info {
	struct multiboot_tag_base base;
	u64 addr;
	u32 pitch;
	u32 width;
	u32 height;
	u8 bpp;
	u8 type;
	u8 reserved;
	union {
		struct multiboot_palette palette;
		struct multiboot_direct_rgb direct;
	} color_info;
};

#define MULTIBOOT_TAG_SIZE(tag) ((((struct multiboot_tag_base*) tag)->size + 7) & ~7)
#define MULTIBOOT_NEXT_TAG(tag) ((struct multiboot_tag_base*) ((u8*) tag + MULTIBOOT_TAG_SIZE(tag)))

extern const struct multiboot_info *_multiboot_data;

#endif

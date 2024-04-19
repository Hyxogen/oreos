#ifndef OREOS_MULTIBOOT2
#define OREOS_MULTIBOOT2

#include <kernel/types.h>

#define MULTIBOOT_TAG_TYPE_END 0
#define MULTIBOOT_TAG_TYPE_FRAMEBUFFER 8

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

#endif

#ifndef OREOS_MULTIBOOT2
#define OREOS_MULTIBOOT2

#include <kernel/types.h>

#define MB2_TAG_TYPE_END 0
#define MB2_TAG_TYPE_MMAP 6
#define MB2_TAG_TYPE_FRAMEBUF 8

#define MB2_MMAP_TYPE_AVAIL 1
#define MB2_MMAP_TYPE_ACPI_RECLAIMABLE 3
#define MB2_MMAP_TYPE_RESERVED_HIBER 4
#define MB2_MMAP_TYPE_BADRAM 5

#define FRAMEBUFFER_TYPE_PALETTE 0
#define FRAMEBUFFER_TYPE_DIRECT 1
#define FRAMEBUFFER_TYPE_EGA	2

struct mb2_info {
	u32 total_size;
	u32 reserved;
	u8 tags[] __attribute__((aligned(8)));
};

struct mb2_tag_base {
	u32 type;
	u32 size;
};

struct mb2_color_desc {
	u8 red;
	u8 green;
	u8 blue;
};

struct mb2_palette {
	u32 num_colors;
	struct mb2_color_desc palette[];
};

struct mb2_direct_color {
	u8 field_pos;
	u8 mask_size;
};

struct mb2_direct_rgb {
	union {
		struct {
			struct mb2_direct_color red;
			struct mb2_direct_color green;
			struct mb2_direct_color blue;
		};
		struct mb2_direct_color colors[3];
	};
};

struct mb2_mmap_entry {
	u64 base_addr;
	u64 length;
	u32 type;
	u32 reserved;
};

struct mb2_mmap {
	struct mb2_tag_base base;
	u32 entry_size;
	u32 entry_version;
	u8 entries[];
};

struct mb2_framebuf_info {
	struct mb2_tag_base base;
	u64 addr;
	u32 pitch;
	u32 width;
	u32 height;
	u8 bpp;
	u8 type;
	u8 reserved;
	union {
		struct mb2_palette palette;
		struct mb2_direct_rgb direct;
	} color_info;
};

#define MB2_TAG_SIZE(tag) ((((struct mb2_tag_base*) tag)->size + 7) & ~7)
#define MB2_NEXT_TAG(tag) ((struct mb2_tag_base*) ((u8*) tag + MB2_TAG_SIZE(tag)))

struct mb2_tag_base *mb2_find(const struct mb2_info *info, u32 type);

#endif

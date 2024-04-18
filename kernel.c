#include "types.h"
#include "multiboot2.h"

extern void halt(void);

__attribute__ ((noinline)) void panic(void)
{
	halt();
}

struct color_desc {
	u8 field_pos;
	u8 mask_size;
};

struct color {
	union {
		struct {
			u8 red;
			u8 green;
			u8 blue;
		};
		u8 elems[3];
	};
};

static volatile u8* term_buf;
static u32 term_width;
static u32 term_height;
static u8 term_bpp;
static struct color_desc colors[3];

static const struct color COLOR_PURPLE = {{{ 0xff, 0x00, 0xff }}};
static const struct color COLOR_BLACK = {{{ 0x00, 0x00, 0x00 }}};

size_t strlen(const char *str)
{
	const char *beg = str;

	while (*str)
		++str;
	return str - beg;
}

u32 term_encode_color(struct color color)
{
	u32 result = 0;

	for (int i = 0; i < 3; ++i) {
		result |= ((u32) color.elems[i] & ((1 << colors[i].mask_size) - 1)) << colors[i].field_pos;
	}
	return result;
}

void term_putat(size_t x, size_t y, struct color color)
{
	u32 col = term_encode_color(color);

	*(u32*) &term_buf[y * term_height * (term_bpp >> 3) + x * (term_bpp >> 3)] = col;
}

void term_clear(struct color color)
{
	for (u32 y = 0; y < term_height; ++y) {
		for (u32 x = 0; x < term_width; ++x) {
			term_putat(x, y, color);
		}
	}
}

void term_init(const struct multiboot_info *info)
{
	const struct multiboot_tag_base *tag = (const struct multiboot_tag_base*) info->tags;
	for (; tag->type != MULTIBOOT_TAG_TYPE_END; tag = MULTIBOOT_NEXT_TAG(tag)) {
		if (tag->type == MULTIBOOT_TAG_TYPE_FRAMEBUFFER) {
			struct multiboot_framebuffer_info *f = (struct multiboot_framebuffer_info*) tag;

			if (f->type != FRAMEBUFFER_TYPE_DIRECT || f->bpp > 32)
				panic(); //unsupported framebuffer type or bpp

			for (int i = 0; i < 3; ++i) {
				colors[i].field_pos = f->color_info.direct.colors[i].field_pos;
				colors[i].mask_size = f->color_info.direct.colors[i].mask_size;
			}

			term_buf = (u8*) (uintptr_t) f->addr;
			term_width = f->width;
			term_height = f->height;
			term_bpp = f->bpp;

			term_clear(COLOR_PURPLE);
			/*for (u32 y = 0; y < f->height; ++y) {
				for (u32 x = 0; x < f->width; ++x) {
					term_putat(x, y, COLOR_PURPLE);
				}
			}*/
			return;
		}
	}
	//no framebuffer
	panic();
}

void kernel_main(struct multiboot_info *info)
{
	term_init(info);

	//term_print("Hello World!\n");
}

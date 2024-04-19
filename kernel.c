#include "types.h"
#include "multiboot2.h"
#include "psf.h"
#include <stdbool.h>

__attribute__ ((noreturn)) extern void halt(void);

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
static const struct color COLOR_WHITE = {{{ 0xff, 0xff, 0xff }}};

extern struct psf2_font _binary_font_psfu_start;
extern u8 _binary_font_psfu_end;
static u16 unicode[256];

static void psf_init()
{
	//todo check magic
	struct psf2_font* font = (struct psf2_font*) &_binary_font_psfu_start;
	if (font->hdr.flags) {
		for (int i = 0; i < 256; ++i) {
			unicode[i] = i;
		}
		return;
	}

	u8* s = font->data + font->hdr.count * font->hdr.charsize;
	u8 glyph = 0;
	while (s < (u8*) &_binary_font_psfu_end) {
		u8 b = *s;

		if (b == 0xFF) {
			++glyph;
		} else if ((b & 0xF8) == 0xF0) {
			s += 3;
		} else if ((b & 0xF0) == 0xD0) {
			s += 2;
		} else if ((b & 0xD0) == 0xC0) {
			s += 1;
		} else if ((b & 0x80) == 0x00) {
			unicode[b] = glyph;
		} else {
			//invalid utf8
			panic();
		}

		++s;
	}
}

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

static u32* term_get_pixel(u32 x, u32 y)
{

	return (u32*) &term_buf[y * term_width * (term_bpp >> 3) + x * (term_bpp >> 3)];
}

static void term_putat_encoded(u32 x, u32 y, u32 color)
{
	*term_get_pixel(x, y) = color;
}

void term_putat(u32 x, u32 y, struct color color)
{
	term_putat_encoded(x, y, term_encode_color(color));
}

void term_putchar(u32 cx, u32 cy, char c, struct color fg, struct color bg)
{
	struct psf2_font* font = (struct psf2_font*) &_binary_font_psfu_start;

	u32 f = term_encode_color(fg);
	u32 b = term_encode_color(bg);

	u8* glyph = font->data + unicode[(u8) c] * font->hdr.charsize;

	u32 bytes_per_row = (font->hdr.width + 7) / 8;

	u32 xoff = cx * font->hdr.width;
	u32 yoff = cy * font->hdr.height;

	for (u32 y = 0; y < font->hdr.height; ++y) {
		u8* row = &glyph[y * bytes_per_row];

		for (u32 x = 0; x < font->hdr.width; ++x) {
			u8 col = row[x / 8];
			u8 mask = 0x80 >> (x % 8);

			term_putat_encoded(x + xoff, y + yoff, (col & mask) ? f : b);
		}
	}
}

void term_clear(struct color color)
{
	u32 col = term_encode_color(color);
	for (u32 y = 0; y < term_height; ++y) {
		for (u32 x = 0; x < term_width; ++x) {
			//TODO make this not use term_putat as it is not
			//necesarry to recalculate x,y internally every call
			term_putat_encoded(x, y, col);
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

			//term_clear(COLOR_BLACK);
			return;
		}
	}
	//no framebuffer
	panic();
}

void kernel_main(struct multiboot_info *info)
{
	psf_init();
	term_init(info);

	term_putchar(1, 1, 'x', COLOR_WHITE, COLOR_BLACK);
	//term_print("Hello World!\n");
}

#include <stdbool.h>

#include <kernel/tty.h>
#include <kernel/types.h>
#include <kernel/kernel.h>
#include "multiboot2.h"
#include "psf.h"
#include "boot.h"

static bool initialized = false;
static u8* term_buf;
static u32 term_width, term_height;
static u32 term_charwidth, term_charheight;
static u32 term_col, term_row;
static u8 term_bpp;
static u32 term_fg, term_bg;
static struct {
	u8 field_pos;
	u8 mask_size;
} colors[3];

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

static const struct color COLOR_PURPLE = {{{ 0xff, 0x00, 0xff }}};
static const struct color COLOR_BLACK = {{{ 0x00, 0x00, 0x00 }}};
static const struct color COLOR_WHITE = {{{ 0xff, 0xff, 0xff }}};

extern struct psf2_font _binary_font_psfu_start;
extern u8 _binary_font_psfu_end;
static u16 unicode[256];

u32 term_encode_color(struct color color)
{
       u32 result = 0;

       for (int i = 0; i < 3; ++i) {
               result |= ((u32) color.elems[i] & ((1 << colors[i].mask_size) - 1)) << colors[i].field_pos;
       }
       return result;
}


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
			continue;
		}

		++s;
	}
}

static u32* term_get_pixel(u32 x, u32 y)
{

	return (u32*) &term_buf[y * term_width * (term_bpp >> 3) + x * (term_bpp >> 3)];
}

static void term_putat_encoded(u32 x, u32 y, u32 color)
{
	*term_get_pixel(x, y) = color;
}

void term_putcharat(u32 cx, u32 cy, char c, u32 fg, u32 bg)
{
	struct psf2_font* font = (struct psf2_font*) &_binary_font_psfu_start;

	u8* glyph = font->data + unicode[(u8) c] * font->hdr.charsize;

	u32 bytes_per_row = (font->hdr.width + 7) / 8;

	u32 xoff = cx * font->hdr.width;
	u32 yoff = cy * font->hdr.height;

	for (u32 y = 0; y < font->hdr.height; ++y) {
		u8* row = &glyph[y * bytes_per_row];

		for (u32 x = 0; x < font->hdr.width; ++x) {
			u8 col = row[x / 8];
			u8 mask = 0x80 >> (x % 8);

			term_putat_encoded(x + xoff, y + yoff, (col & mask) ? fg : bg);
		}
	}
}

static void term_clear(struct color color)
{
	u32 col = term_encode_color(color);

	for (u32 y = 0; y < term_height; ++y) {
		for (u32 x = 0; x < term_width; ++x) {
			term_putat_encoded(x, y, col);
		}
	}
}

void term_put(int c)
{
	term_putcharat(term_col, term_row, c, term_fg, term_bg);
	if (++term_col == term_width) {
		term_col = 0;
		if (++term_row == term_height) {
			term_row = 0;
		}
	}
}

void term_write(const char *data, size_t n)
{
	for (size_t i = 0; i < n; ++i)
		term_put(data[i]);
}

static size_t _strlen(const char *str)
{
	const char *beg = str;

	while (*str)
		++str;

	return str - beg;
}

void term_print(const char *str)
{
	term_write(str, _strlen(str));
}

void _term_init(const struct multiboot_info *boot_info)
{
	if (initialized)
		return;

	psf_init();

	const struct multiboot_tag_base *tag = (const struct multiboot_tag_base*) boot_info->tags;

	for (; tag->type != MULTIBOOT_TAG_TYPE_END; tag = MULTIBOOT_NEXT_TAG(tag)) {
		if (tag->type == MULTIBOOT_TAG_TYPE_FRAMEBUFFER) {
			struct multiboot_framebuffer_info *f = (struct multiboot_framebuffer_info*) tag;

			if (f->type != FRAMEBUFFER_TYPE_DIRECT || f->bpp > 32)
				panic(""); //unsupported framebuffer type or bpp

			for (int i = 0; i < 3; ++i) {
				colors[i].field_pos = f->color_info.direct.colors[i].field_pos;
				colors[i].mask_size = f->color_info.direct.colors[i].mask_size;
                        }

                        term_buf = (u8*) (uintptr_t) f->addr;
			term_width = f->width;
			term_height = f->height;
			term_bpp = f->bpp;

			struct psf2_font* font = (struct psf2_font*) &_binary_font_psfu_start;
			term_charwidth = term_width / font->hdr.width;
			term_charheight = term_height / font->hdr.height;
			term_row = term_col = 0;
			term_fg = term_encode_color(COLOR_WHITE);
			term_bg = term_encode_color(COLOR_BLACK);

			term_clear(COLOR_BLACK);
			initialized = true;
			return;
		}
	}
	//no framebuffer
	panic("");
}


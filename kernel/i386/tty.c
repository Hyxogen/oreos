#include <stdbool.h>

#include <kernel/tty.h>
#include <kernel/types.h>
#include <kernel/kernel.h>
#include <libc/string.h>
#include <libc/ctype.h>
#include <libc/stdlib.h>
#include "multiboot2.h"
#include "psf.h"

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

#define COLOR_BLACK 0
#define COLOR_RED 1
#define COLOR_GREEN 2
#define COLOR_YELLOW 3
#define COLOR_BLUE 4
#define COLOR_MAGENTA 5
#define COLOR_CYAN 6
#define COLOR_WHITE 7

static const struct color ANSI_COLORS[] = {
	{{{0, 0, 0}}}, // Black
	{{{170, 0, 0}}}, // Red
	{{{0, 170, 0}}}, // Green
	{{{170, 85, 0}}}, // Yellow
	{{{0, 0, 170}}}, // Blue
	{{{170, 0, 170}}}, // Magenta
	{{{0, 170, 170}}}, // Cyan
	{{{170, 170, 170}}}, // White
};

extern struct psf2_font _binary_font_psfu_start;
extern u8 _binary_font_psfu_end;
static u16 unicode[256];
static struct psf2_font* font = (struct psf2_font*) &_binary_font_psfu_start;

static u32 term_encode_color(struct color color)
{
       u32 result = 0;

       for (int i = 0; i < 3; ++i) {
               result |= ((u32) color.elems[i] & ((1 << colors[i].mask_size) - 1)) << colors[i].field_pos;
       }
       return result;
}


//https://www.win.tue.nl/~aeb/linux/kbd/font-formats-1.html
static void psf_init()
{
	//todo check magic
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

static void term_putcharat(u32 cx, u32 cy, char c, u32 fg, u32 bg)
{
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


static void term_fillchar(u32 cx, u32 cy, u32 color)
{
	u32 xoff = cx * font->hdr.width;
	u32 yoff = cy * font->hdr.height;

	for (u32 y = 0; y < font->hdr.height; ++y) {
		for (u32 x = 0; x < font->hdr.width; ++x)
			term_putat_encoded(x + xoff, y + yoff, color);
	}

}

static void term_cleararea(u32 x1, u32 y1, u32 x2, u32 y2, u32 color)
{
	for (u32 y = y1; y < y2; ++y) {
		for (u32 x = x1; x < x2; ++x) {
			term_putat_encoded(x, y, color);
		}
	}
}


static void term_clear(u32 color)
{
	term_cleararea(0, 0, term_width, term_height, color);
}

static void term_clearchars(u32 cx1, u32 cy1, u32 cx2, u32 cy2, u32 color)
{
	u32 width = font->hdr.width;
	u32 height = font->hdr.width;
	term_cleararea(cx1 * width, cy1 * height, cx2 * width, cy2 * height, color);
}

static void term_clearlines(u32 cy1, u32 cy2, u32 color)
{
	term_clearchars(0, cy1, term_charwidth, cy2, color);
}

static void term_clearline(u32 cy, u32 color)
{
	term_clearlines(cy, cy + 1, color);
}

static void term_scroll(void)
{
	u32* start = term_get_pixel(0, font->hdr.height);
	u32* end = term_get_pixel(term_width - 1, term_height - 1);
	memmove(term_buf, start, (end - start) * sizeof(u32));

	term_clearline(term_charheight - 1, term_bg);
}

static void term_newline(void)
{
	term_col = 0;
	if (++term_row == term_charheight) {
		term_scroll();
		term_row = term_charheight - 1;
	}
}

static void term_exec_escape(const char *str)
{
	if (*str++ != '[')
		return;

	int i = 0;

	if (isdigit((*str))) {
		i = atoi(str);
		while (isdigit(*str))
			str += 1;
	}

	switch (*str) {
	case 'H':
		term_col = 0;
		term_row = 0;
		return;
	case 'J':
		term_clearlines(term_row + 1, term_charheight, term_bg);
		__attribute__ ((fallthrough));
	case 'K':
		term_clearchars(term_col, term_row, term_width, term_row + 1, term_bg);
		return;
	case 'm':
		if (i == 0) {
			term_fg = term_encode_color(ANSI_COLORS[COLOR_WHITE]);
			term_bg = term_encode_color(ANSI_COLORS[COLOR_BLACK]);
		} else if (i >= 30 && i <= 37) {
			term_fg = term_encode_color(ANSI_COLORS[COLOR_RED]);
			//term_fg = term_encode_color(ANSI_COLORS[i - 30]);
		} else if (i >= 40 && i < 47) {
			term_bg = term_encode_color(ANSI_COLORS[i - 40]);
		}
		break;
	default:
		if (isalpha(*str))
			return; // not supported
	}
}

static bool term_put_escaped(int c)
{
	static char buf[32];
	static size_t i = 0;

	if (i < (ARRAY_SIZE(buf) - 1))
		buf[i++] = (char) c;
	else
		return false; // too long

	if (isalpha(c) || c == '~') {
		//end of escape seq
		buf[i] = '\0';
		term_exec_escape(buf);
		return false;
	} 
	return true;
}

void term_put(int c)
{
	if (!c)
		return;

	static bool escaped = false;

	if (c == ANSI_ESCAPE) {
		escaped = true;
	} else if (escaped) {
		escaped = term_put_escaped(c);
	} else if (c == '\n') {
		term_fillchar(term_col, term_row, term_bg);
		term_newline();
	} else if (c == '\b') {
		term_fillchar(term_col, term_row, term_bg);
		if (term_col-- == 0)
			term_col = 0;
		term_putcharat(term_col, term_row, ' ', term_fg, term_bg);
	} else {
		term_putcharat(term_col, term_row, c, term_fg, term_bg);
		if (++term_col == term_charwidth) {
			term_col = 0;

			term_newline();
		}
	}
	term_fillchar(term_col, term_row, term_fg);
}

void term_write(const char *data, size_t n)
{
	for (size_t i = 0; i < n; ++i)
		term_put(data[i]);
}

void term_print(const char *str)
{
	term_write(str, strlen(str));
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

			term_charwidth = term_width / font->hdr.width;
			term_charheight = term_height / font->hdr.height;
			term_row = term_col = 0;
			term_fg = term_encode_color(ANSI_COLORS[COLOR_WHITE]);
			term_bg = term_encode_color(ANSI_COLORS[COLOR_BLACK]);

			term_clear(term_bg);
			initialized = true;
			return;
		}
	}
	//no framebuffer
	panic("");
}


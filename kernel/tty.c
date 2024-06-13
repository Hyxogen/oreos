#include <kernel/framebuf.h>
#include <kernel/kernel.h>
#include <kernel/psf.h>
#include <kernel/tty.h>
#include <libc/ctype.h>
#include <libc/stdlib.h>
#include <libc/string.h>
#include <stdbool.h>

#define TTY_COLOR_BLACK 0
#define TTY_COLOR_RED 1
#define TTY_COLOR_GREEN 2
#define TTY_COLOR_YELLOW 3
#define TTY_COLOR_BLUE 4
#define TTY_COLOR_MAGENTA 5
#define TTY_COLOR_CYAN 6
#define TTY_COLOR_WHITE 7

extern const struct psf2_font _binary_font_psfu_start;
extern const u8 _binary_font_psfu_end;

static bool term_initialized;
static struct term term_main;

struct term *term_get_primary(void)
{
	if (!term_initialized) {
		term_initialized = true;

		term_init(&term_main, fb_get_primary());
	}
	return &term_main;

}

void term_init(struct term *term, struct framebuf *fb)
{
	term->row = 0;
	term->col = 0;
	term->fg_color = TTY_COLOR_WHITE;
	term->bg_color = TTY_COLOR_BLACK;
	term->_fb = fb;
	term->_escape = -1;

	font_read_from(&term->_font, &_binary_font_psfu_start,
		       &_binary_font_psfu_end -
			   (unsigned char *)&_binary_font_psfu_start);

	term->width = fb->width / term->_font.psf->hdr.width;
	term->height = fb->height / term->_font.psf->hdr.height;

	if (term->width > TERM_WIDTH)
		term->width = TERM_WIDTH;
	if (term->height > TERM_HEIGHT)
		term->height = TERM_HEIGHT;

	for (unsigned i = 0; i < ARRAY_SIZE(term->chars); i++)
		term->chars[i] =
		    (struct term_char){term->fg_color, term->bg_color, ' '};
}

static struct fb_color term_conv_color(u8 col)
{
	switch (col) {
	case TTY_COLOR_BLACK:
		return FB_COLOR_BLACK;
	case TTY_COLOR_RED:
		return FB_COLOR_RED;
	case TTY_COLOR_GREEN:
		return FB_COLOR_GREEN;
	case TTY_COLOR_YELLOW:
		return FB_COLOR_YELLOW;
	case TTY_COLOR_BLUE:
		return FB_COLOR_BLUE;
	case TTY_COLOR_MAGENTA:
		return FB_COLOR_MAGENTA;
	case TTY_COLOR_CYAN:
		return FB_COLOR_CYAN;
	default:
	case TTY_COLOR_WHITE:
		return FB_COLOR_WHITE;
	}
}

static struct term_char *term_get_char(struct term *term, int row, int col)
{
	return &term->chars[row * term->width + col];
}

static void term_putat(struct term *term, int row, int col, struct term_char ch)
{
	struct font *font = &term->_font;

	const u8 *glyph = font_get_glyph(font, ch.codepoint);

	u32 bytes_per_row = (font->psf->hdr.width + 7) / 8;

	u32 xoff = col * font->psf->hdr.width;
	u32 yoff = row * font->psf->hdr.height;

	struct fb_color enc_fg_color = term_conv_color(ch.fg_color);
	struct fb_color enc_bg_color = term_conv_color(ch.bg_color);

	*term_get_char(term, row, col) = ch;

	for (u32 y = 0; y < font->psf->hdr.height; ++y) {
		const u8 *row = &glyph[y * bytes_per_row];

		for (u32 x = 0; x < font->psf->hdr.width; ++x) {
			u8 col = row[x / 8];
			u8 mask = 0x80 >> (x % 8);

			fb_put(term->_fb, x + xoff, y + yoff,
			       (col & mask) ? enc_fg_color : enc_bg_color);
		}
	}
}

static void term_clearat(struct term *term, int row, int col, int color)
{
	term_putat(term, row, col,
		   (struct term_char){term->fg_color, color, ' '});
}

static void term_reset_escape(struct term *term) { term->_escape = -1; }

static void term_exec_escape(struct term *term, const char *str)
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
		term->col = 0;
		term->row = 0;
		return;
	case 'm':
		if (i == 0) {
			term->fg_color = TTY_COLOR_WHITE;
			term->bg_color = TTY_COLOR_BLACK;
		} else if (i >= 30 && i <= 37) {
			term->fg_color = i - 30;
		} else if (i >= 40 && i < 47) {
			term->bg_color = i - 40;
		}
		break;
	default:
		if (isalpha(*str))
			break; // not supported
	}
}

static void term_put_escaped(struct term *term, int c)
{
	if (term->_escape < (int)(ARRAY_SIZE(term->_escape_buf) - 1)) {
		term->_escape_buf[term->_escape++] = (char)c;
	} else {
		term_reset_escape(term);
		return; // too long;
	}

	if (isalpha(c) || c == '~') {
		// end of escape sequence
		term->_escape_buf[term->_escape] = '\0';
		term_exec_escape(term, term->_escape_buf);
		term_reset_escape(term);
	}
}

void term_redraw(struct term *term)
{
	for (int row = 0; row < term->height; row++) {
		for (int col = 0; col < term->width; col++) {
			term_putat(term, row, col,
				   *term_get_char(term, row, col));
		}
	}
}

static void term_scroll(struct term *term)
{
	memmove(term->chars, term_get_char(term, 1, 0),
		(ARRAY_SIZE(term->chars) - term->width) *
		    sizeof(term->chars[0]));
	term_redraw(term);
}

static void term_newline(struct term *term)
{
	term->col = 0;
	if (++term->row == term->height) {
		term_scroll(term);
		term->row -= 1;
	}
}

void term_put(struct term *term, int ch)
{
	if (!ch)
		return;

	if (term->_escape >= 0) {
		term_put_escaped(term, ch);
	} else if ((char)ch == TTY_ESCAPE) {
		term->_escape = 0;
	} else if ((char)ch == '\n') {
		term_clearat(term, term->row, term->col, term->bg_color);
		term_newline(term);
	} else if ((char)ch == '\b') {
		term_clearat(term, term->row, term->col, term->bg_color);
		if (term->col-- == 0)
			term->col = 0;
	} else {
		term_putat(term, term->row, term->col,
			   (struct term_char){term->fg_color, term->bg_color,
					      (u32)ch});
		if (++term->col == term->width) {
			term_newline(term);
		}
	}
	term_clearat(term, term->row, term->col, term->fg_color);
}

void term_write(struct term *term, const char *data, size_t n)
{
	for (size_t i = 0; i < n; ++i)
		term_put(term, data[i]);
}

void term_print(struct term *term, const char *str)
{
	term_write(term, str, strlen(str));
}

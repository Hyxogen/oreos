#ifndef __KERNEL_TTY_H
#define __KERNEL_TTY_H

#include <kernel/psf.h>
#include <kernel/types.h>

#define TERM_WIDTH 80
#define TERM_HEIGHT 24

#define TTY_ESCAPE 033

struct term_char {
	u8 fg_color : 4;
	u8 bg_color : 4;
	u8 codepoint;
};

struct term {
	int width;
	int height;
	int row;
	int col;
	int fg_color;
	int bg_color;
	int _escape;
	char _escape_buf[32];

	struct font _font;
	struct framebuf *_fb;

	struct term_char chars[TERM_WIDTH * TERM_HEIGHT];
};

struct term *term_get_primary(void);
void term_init(struct term *term, struct framebuf *fb);
void term_put(struct term *term, int c);
void term_write(struct term *term, const char *data, size_t n);
void term_print(struct term *term, const char *str);
void term_redraw(struct term *term);

#endif

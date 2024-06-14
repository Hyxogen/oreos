#ifndef KERNEL_FRAMEBUF_H
#define KERNEL_FRAMEBUF_H

#include <kernel/types.h>

struct fb_color {
	union {
		struct {
			u8 red;
			u8 green;
			u8 blue;
		};
		u8 elems[3];
	};
};

#define FB_COLOR_BLACK ((struct fb_color){{{0, 0, 0}}})
#define FB_COLOR_RED ((struct fb_color){{{170, 0, 0}}})
#define FB_COLOR_GREEN ((struct fb_color){{{0, 170, 0}}})
#define FB_COLOR_YELLOW ((struct fb_color){{{170, 85, 0}}})
#define FB_COLOR_BLUE ((struct fb_color){{{0, 0, 170}}})
#define FB_COLOR_MAGENTA ((struct fb_color){{{170, 0, 170}}})
#define FB_COLOR_CYAN ((struct fb_color){{{0, 170, 170}}})
#define FB_COLOR_WHITE ((struct fb_color){{{170, 170, 170}}})

struct framebuf {
	void *data;
	u32 width;
	u32 height;
	u8 bpp;

	struct {
		u8 field_pos;
		u8 mask_size;
	} _colors[3];
};

struct framebuf *fb_get_primary();
// anchor is top left
void fb_draw_rect(struct framebuf *buf, u32 x, u32 y, u32 width, u32 height,
		  struct fb_color);
void fb_clear(struct framebuf *buf, struct fb_color color);
void fb_put(struct framebuf *buf, u32 x, u32 y, struct fb_color color);

#endif

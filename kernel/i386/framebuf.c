#include <kernel/kernel.h>
#include <kernel/framebuf.h>
#include "multiboot2.h"
#include <stdbool.h>
#include <libc/assert.h>

static bool term_initialized;
static struct framebuf fb_main;

static u32* fb_get_pixel_addr(struct framebuf *fb, u32 x, u32 y)
{
	u8 *buf = fb->data;
	u8 tmp = fb->bpp >> 3;
	return (u32*) &buf[y * fb->width * tmp + x * tmp];
}

static u32 fb_encode_color(const struct framebuf *fb, struct fb_color color)
{
       u32 result = 0;

       for (int i = 0; i < 3; ++i) {
               result |= ((u32) color.elems[i] & ((1 << fb->_colors[i].mask_size) - 1)) << fb->_colors[i].field_pos;
       }
       return result;
}

struct framebuf *fb_get_primary()
{
	if (!term_initialized)
		return NULL;
	return &fb_main;
}

void fb_draw_rect(struct framebuf *fb, u32 x, u32 y, u32 width, u32 height, struct fb_color color)
{
	u32 enc = fb_encode_color(fb, color);
	assert(x + width <= fb->width);
	assert(y + height <= fb->height);

	for (u32 yoff = 0; yoff < height; yoff++) {
		for (u32 xoff = 0; xoff < width; xoff++) {
			*fb_get_pixel_addr(fb, x + xoff, y + yoff) = enc;
		}
	}
}

void fb_clear(struct framebuf *fb, struct fb_color color)
{
	fb_draw_rect(fb, 0, 0, fb->width, fb->height, color);
}

void fb_put(struct framebuf *fb, u32 x, u32 y, struct fb_color color)
{
	*fb_get_pixel_addr(fb, x, y) = fb_encode_color(fb, color);
}

void _term_init(const struct multiboot_info *boot_info)
{
	if (term_initialized)
		return;

	const struct multiboot_tag_base *tag = (const struct multiboot_tag_base*) boot_info->tags;

	for (; tag->type != MULTIBOOT_TAG_TYPE_END; tag = MULTIBOOT_NEXT_TAG(tag)) {
		if (tag->type == MULTIBOOT_TAG_TYPE_FRAMEBUFFER) {
			struct multiboot_framebuffer_info *f = (struct multiboot_framebuffer_info*) tag;

			if (f->type != FRAMEBUFFER_TYPE_DIRECT || f->bpp > 32)
				panic(""); //unsupported framebuf type or bpp

			for (int i = 0; i < 3; ++i) {
				fb_main._colors[i].field_pos = f->color_info.direct.colors[i].field_pos;
				fb_main._colors[i].mask_size = f->color_info.direct.colors[i].mask_size;
                        }

                        fb_main.data = (u8*) (uintptr_t) f->addr;
			fb_main.width = f->width;
			fb_main.height = f->height;
			fb_main.bpp = f->bpp;
			fb_clear(&fb_main, FB_COLOR_BLACK);

			term_initialized = true;
			return;
		}
	}
	//no framebuf
	panic("");
}

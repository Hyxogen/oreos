#include <boot/multiboot2.h>
#include <kernel/align.h>
#include <kernel/framebuf.h>
#include <kernel/kernel.h>
#include <kernel/mmu.h>
#include <kernel/libc/assert.h>
#include <stdbool.h>
#include <stddef.h>
#include <kernel/debug.h>

static bool term_initialized;
static struct framebuf fb_main;

static u32 *fb_get_pixel_addr(struct framebuf *fb, u32 x, u32 y)
{
	u8 *buf = fb->data;
	u8 tmp = fb->bpp >> 3;
	return (u32 *)&buf[y * fb->width * tmp + x * tmp];
}

static u32 fb_encode_color(const struct framebuf *fb, struct fb_color color)
{
	u32 result = 0;

	for (int i = 0; i < 3; ++i) {
		result |= ((u32)color.elems[i] &
			   ((1 << fb->_colors[i].mask_size) - 1))
			  << fb->_colors[i].field_pos;
	}
	return result;
}

struct framebuf *fb_get_primary()
{
	if (!term_initialized)
		return NULL;
	return &fb_main;
}

void fb_draw_rect(struct framebuf *fb, u32 x, u32 y, u32 width, u32 height,
		  struct fb_color color)
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

//0xfd00 0000 (phys)
void init_framebuf(struct mb2_info *mb)
{
	if (term_initialized)
		return;

	struct mb2_framebuf_info *f =
	    (struct mb2_framebuf_info *)mb2_find(mb, MB2_TAG_TYPE_FRAMEBUF);
	if (!f)
		panic(""); // no framebuffer present

	if (f->type != FRAMEBUFFER_TYPE_DIRECT || f->bpp > 32)
		panic(""); // unsupported framebuf type or bpp

	for (int i = 0; i < 3; ++i) {
		fb_main._colors[i].field_pos =
		    f->color_info.direct.colors[i].field_pos;
		fb_main._colors[i].mask_size =
		    f->color_info.direct.colors[i].mask_size;
	}

	size_t total_size = f->width * f->height * (f->bpp / 8);

	fb_main.data =
	    mmu_map(NULL, f->addr, total_size, MMU_ADDRSPACE_KERNEL, 0);
	fb_main.width = f->width;
	fb_main.height = f->height;
	fb_main.bpp = f->bpp;
	fb_clear(&fb_main, FB_COLOR_BLACK);

	term_initialized = true;
	return;
}

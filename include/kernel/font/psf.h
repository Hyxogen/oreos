#ifndef __KERNEL_PSF
#define __KERNEL_PSF

#include <kernel/types.h>
#include <stddef.h>

// TODO check endianes
#define PSF2_MAGIC 0x72b54a86

extern unsigned char psf2_default_font[];
extern unsigned int psf2_default_font_len;

struct psf2_hdr {
	u32 magic;
	u32 version;
	u32 hdrsize;
	u32 flags;
	u32 count;
	u32 charsize;
	u32 height;
	u32 width;
};

struct psf2_font {
	struct psf2_hdr hdr;
	u8 data[];
};

struct font {
	const struct psf2_font *psf;
	u8 ascii[256];
	size_t size;
};

int font_read_from(struct font *dest, const void *src, size_t size);
const u8 *font_get_glyph(struct font *font, u32 codepoint);

#endif

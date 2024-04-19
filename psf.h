#ifndef OREOS_PSF
#define OREOS_PSF

#include "types.h"

//TODO check endianes
#define PSF2_MAGIC 0x72b54a86

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

#endif

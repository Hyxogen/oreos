#ifndef __KERNEL_ALIGN_H
#define __KERNEL_ALIGN_H

#define __ALIGN_UP_MASK(x, mask) (((x) + (mask)) & ~(mask))

#define ALIGN_UP(x, boundary) __ALIGN_UP_MASK((x), (__typeof__(x))(boundary) - 1)
#define ALIGN_DOWN(x, boundary) ALIGN_UP((x) - ((boundary) - 1), boundary) 

#define PTR_ALIGN_UP(p, boundary) \
	((__typeof__(p)) ALIGN_UP((uintptr_t)(p), (boundary)))
#define PTR_ALIGN_DOWN(p, boundary) \
	((__typeof__(p)) ALIGN_DOWN((uintptr_t)(p), (boundary)))

#define IS_ALIGNED(p, boundary) (((p) & ((__typeof__(boundary))(boundary) - 1)) == 0)
#define PTR_IS_ALIGNED(p, boundary) IS_ALIGNED((uintptr_t)(p), (boundary))

#endif

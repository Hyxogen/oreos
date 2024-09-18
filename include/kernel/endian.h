#ifndef __KERNEL_ENDIAN_H
#define __KERNEL_ENDIAN_H

#ifndef __BYTE_ORDER__
# error "__BYTE_ORDER__ must be defined to use this header"
#else
#endif

#include <kernel/types.h>

#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
static inline u16 le_u16(u16 v)
{
	return v;
}
static inline u32 le_u32(u32 v)
{
	return v;
}
#elif __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
# error "TODO"
#else
# error "unsupported endiannes"
#endif

#endif

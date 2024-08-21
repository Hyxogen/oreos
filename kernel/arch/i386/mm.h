#ifndef __KERNEL_ARCH_I386_MM_H
#define __KERNEL_ARCH_I386_MM_H

#include <stddef.h>
#include <stdint.h>
#include <kernel/types.h>

#define MM_MAP_FAILED ((void*) -1)
#define MM_PAGESIZE 0x1000

struct mb2_info *mm_init(struct mb2_info *mb, size_t mb_size);
void *mm_map_physical(uintptr_t paddr, size_t count, u32 flags);

#define MM_MAP_RDONLY 0x1
#define MM_MAP_NOALLOC 0x2

#endif

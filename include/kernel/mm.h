#ifndef __KERNEL_MM_H
#define __KERNEL_MM_H

#include <stddef.h>
#include <boot/multiboot2.h>

#define MM_MAP_RDONLY 0x1
#define MM_MAP_NOALLOC 0x2

#define MM_MAP_FAILED ((void*) -1)
#define MM_PAGESIZE 0x1000


void mm_flush_tlb(void);
void *mm_map_physical(uintptr_t paddr, size_t count, u32 flags);

void init_mm(const struct mb2_info *info);

#endif

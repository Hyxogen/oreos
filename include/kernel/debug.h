#ifndef __KERNEL_DEBUG_H
#define __KERNEL_DEBUG_H

#define BOCHS_BREAK __asm__ volatile("xchg %bx, %bx");

#endif

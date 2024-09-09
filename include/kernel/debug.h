#ifndef __KERNEL_DEBUG_H
#define __KERNEL_DEBUG_H

#include <kernel/platform.h>

#define BOCHS_BREAK __asm__ volatile("xchg %bx, %bx");

void dump_stacktrace_at(const struct cpu_state *state);
void dump_state(const struct cpu_state *state);

#endif

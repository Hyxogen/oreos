#ifndef __KERNEL_SIGNAL_H
#define __KERNEL_SIGNAL_H

#include <stdbool.h>

#define SIGSEGV 11

bool is_valid_signal(int signum);

#endif

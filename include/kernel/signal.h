#ifndef __KERNEL_SIGNAL_H
#define __KERNEL_SIGNAL_H

#include <stdbool.h>

#define SIGKILL 9
#define SIGSEGV 11
#define SIGALRM 14
#define SIGCHLD 17
#define SIGSTOP 19

bool is_valid_signal(int signum);

#endif

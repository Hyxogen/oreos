#ifndef __LIB_KSTRTOX_H
#define __LIB_KSTRTOX_H

#include <kernel/libc/error.h>

enum lib_error kstrtoul(const char *restrict str, char **restrict endptr, int base, unsigned long *res);

#endif

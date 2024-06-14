#ifndef _LIB_KSTRTOX_H
#define _LIB_KSTRTOX_H

#include <lib/error.h>

enum lib_error kstrtoul(const char *restrict str, char **restrict endptr, int base, unsigned long *res);

#endif

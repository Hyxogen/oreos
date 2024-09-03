#ifndef __LIB_ERROR_H
#define __LIB_ERROR_H

enum lib_error {
	LIB_OK = 0,
	LIB_ERANGE,
	LIB_EINVAL,
};

const char *kstrerr(enum lib_error err);
void kperror(const char *s, enum lib_error err);

#endif

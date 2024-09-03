#include <kernel/libc/ctype.h>
#include <kernel/libc/kstrtox.h>
#include <kernel/libc/strings.h>
#include <limits.h>

static const unsigned char table[] = {
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 0,	1,  2,	3,  4,	5,  6,	7,  8,
    9,	-1, -1, -1, -1, -1, -1, -1, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20,
    21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, -1, -1, -1, -1,
    -1, -1, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26,
    27, 28, 29, 30, 31, 32, 33, 34, 35, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1,
};

static enum lib_error strtox(const char *restrict str, char **restrict endptr,
			     unsigned base, unsigned long long lim,
			     unsigned long long *dest)

{
	if (base > 36 || base == 1)
		return LIB_EINVAL;

	if (endptr)
		*endptr = (char *)str;

	int valid = 0;
	while (isspace(*str))
		++str;

	int neg = 0;
	if (*str == '+' || *str == '-')
		neg = -(*str++ == '-');

	if (base == 16 && !strncasecmp(str, "0x", 2))
		str += 2;
	if (!base) {
		if (!strncasecmp(str, "0x", 2)) {
			base = 16;
			str += 2;
		} else if (*str == '0') {
			base = 8;
		} else {
			base = 10;
		}
	}

	unsigned long long res = 0;
	unsigned char val;
	while ((val = table[(unsigned char)*str]) < base) {
		unsigned long long tmp = res * base;
		if (tmp / base != res)
			return LIB_ERANGE;

		tmp += val;
		if (tmp < val)
			return LIB_ERANGE;

		res = tmp;
		valid = 1;
		++str;
	}

	if (endptr && valid)
		*endptr = (char *)str;

	if (res >= lim) {
		if (!(lim & 1) && !neg) {
			return LIB_ERANGE;
		} else if (res > lim) {
			return LIB_ERANGE;
		}
	}
	*dest = (res ^ neg) - neg;
	return 0;
}

enum lib_error kstrtoul(const char *restrict str, char **restrict endptr,
			int base, unsigned long *res)
{
	unsigned long long tmp;
	enum lib_error err = strtox(str, endptr, base, ULONG_MAX, &tmp);

	if (err != LIB_OK)
		return err;
	if (tmp != (unsigned long)tmp)
		return LIB_ERANGE;
	*res = tmp;
	return 0;
}

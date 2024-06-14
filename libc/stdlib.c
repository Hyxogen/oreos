#include <limits.h>

#include <libc/stdlib.h>
#include <libc/ctype.h>

int atoi(const char *str)
{
	int sign = 1;
	long long v = 0;

	while (isspace(*str))
		str += 1;

	switch (*str) {
	case '-':
		sign = -1;
		__attribute__((fallthrough));
	case '+':
		str += 1;
	}

	while (isdigit(*str)) {
		v = v * 10 + *str - '0';

		if (v > (long long)UINT_MAX)
			return 0;

		str += 1;
	}
	return (int)(v * sign);
}


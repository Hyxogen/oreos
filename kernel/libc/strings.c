#include <libc/strings.h>
#include <libc/ctype.h>

int strcasecmp(const char *s1, const char *s2)
{
	const unsigned char *p1 = (const unsigned char *)s1;
	const unsigned char *p2 = (const unsigned char *)s2;

	int val = 0;
	while ((val = (tolower(*p1) - tolower(*p2))) == 0 && *p1) {
		++p1;
		++p2;
	}
	return val;
}

int strncasecmp(const char *s1, const char *s2, size_t n)
{
	const unsigned char *p1 = (const unsigned char *)s1;
	const unsigned char *p2 = (const unsigned char *)s2;

	int val = 0;
	while (n-- && (val = (tolower(*p1) - tolower(*p2))) == 0 && *p1) {
		++p1;
		++p2;
	}
	return val;
}

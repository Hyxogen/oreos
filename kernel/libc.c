#include <limits.h>

#include <libc/ctype.h>
#include <libc/strings.h>
#include <libc/ctype.h>
#include <libc/string.h>

int isalnum(int c)
{
	return isalpha(c) || isdigit(c);
}

int isalpha(int c)
{
	return isupper(c) || islower(c);
}

int iscntrl(int c)
{
	return (unsigned)c < 0x20 || c == 0x7f;
}

int isdigit(int c)
{
	return (unsigned)c - '0' < 10;
}

int isgraph(int c)
{
	return (unsigned)c - 0x21 < 0x5e;
}

int islower(int c)
{
	return (unsigned)c - 'a' < 26;
}

int isprint(int c)
{
	return (unsigned)c - 0x20 < 0x5f;
}

int ispunct(int c)
{
	return isgraph(c) && !isalnum(c);
}

int isspace(int c)
{
	return c == ' ' || (unsigned)c - '\t' < 5;
}

int isupper(int c)
{
	return (unsigned)c - 'A' < 26;
}

int isxdigit(int c)
{
	return isdigit(c) || ((unsigned)c | 32) - 'a' < 6;
}

int isascii(int c)
{
	return !(c & ~0x7f);
}

int isblank(int c)
{
	return c == ' ' || c == '\t';
}

int toupper(int c)
{
	if (islower(c))
		return c & 0x5f;
	return c;
}

int tolower(int c)
{
	if (isupper(c))
		return c | 0x20;
	return c;
}

void *memcpy(void *restrict dest, const void *restrict src, size_t n)
{
	unsigned char *d = dest;
	const unsigned char *s = src;

	while (n--)
		*d++ = *s++;
	return dest;
}

void *mempcpy(void *restrict dest, const void *restrict src, size_t n)
{
	memcpy(dest, src, n);
	return (unsigned char *)dest + n;
}

void *memccpy(void *restrict dest, const void *restrict src, int c, size_t n)
{
	unsigned char *d = dest;
	const unsigned char *s = src;

	for (; n && (*d = *s) != (unsigned char)c; n--, s++, d++)
		;
	if (n)
		return d + 1;
	return NULL;
}

void *memchr(const void *src, int c, size_t n)
{
	const unsigned char *s = src;

	for (; n && *s != (unsigned char)c; n--, s++)
		;
	if (n)
		return (void *)s;
	return NULL;
}

int memcmp(const void *lhs, const void *rhs, size_t n)
{
	const unsigned char *l = lhs;
	const unsigned char *r = rhs;

	for (; n && *l == *r; n--, l++, r++)
		;
	if (n)
		return (*l > *r) - (*l < *r);
	return 0;
}

void *memmove(void *dest, const void *src, size_t n)
{
	unsigned char *d = dest;
	const unsigned char *s = src;

	if (d <= s) {
		while (n--)
			*d++ = *s++;
	} else {
		while (n--)
			d[n] = s[n];
	}
	return dest;
}

void *memset(void *dest, int c, size_t n)
{
	unsigned char *d = dest;

	while (n--)
		*d++ = (unsigned char)c;
	return dest;
}

char *stpcpy(char *restrict dest, const char *restrict src)
{
	char *p = mempcpy(dest, src, strlen(src));
	*p = '\0';
	return p;
}

char *strcpy(char *restrict dest, const char *restrict src)
{
	stpcpy(dest, src);
	return dest;
}

size_t strspn(const char *dest, const char *src)
{
	const char *tmp = dest;
	size_t src_len = strlen(src);

	while (*tmp) {
		if (!memchr(src, *tmp, src_len))
			break;
		++tmp;
	}
	return tmp - dest;
}

size_t strcspn(const char *dest, const char *src)
{
	const char *tmp = dest;
	size_t src_len = strlen(src);

	while (*tmp) {
		if (memchr(src, *tmp, src_len))
			break;
		++tmp;
	}
	return tmp - dest;
}

size_t strlen(const char *str)
{
	const char *tmp = str;

	while (*tmp)
		++tmp;
	return tmp - str;
}

int strcmp(const char *s1, const char *s2)
{
	const unsigned char *p1 = (const unsigned char *)s1;
	const unsigned char *p2 = (const unsigned char *)s2;

	int val = 0;
	while ((val = (*p1 - *p2)) == 0 && *p1) {
		++p1;
		++p2;
	}
	return val;
}

int strncmp(const char *s1, const char *s2, size_t n)
{
	const unsigned char *p1 = (const unsigned char *)s1;
	const unsigned char *p2 = (const unsigned char *)s2;

	int val = 0;
	while (n-- && (val = (*p1 - *p2)) == 0 && *p1) {
		++p1;
		++p2;
	}
	return val;
}

char *strchr(const char *s, int c)
{
	while (*s != (char)c) {
		if (!*s++)
			return NULL;
	}
	return (char *)s;
}

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

int atoi(const char *str)
{
	int sign = 1;
	long long v = 0;

	while (isspace(*str))
		str += 1;

	switch (*str) {
	case '-':
		sign = -1;
		__attribute__ ((fallthrough));
	case '+':
		str += 1;
	}

	while (isdigit(*str)) {
		v = v * 10 + *str - '0';

		if (v > (long long) UINT_MAX)
			return 0;

		str += 1;
	}
	return (int) (v * sign);
}

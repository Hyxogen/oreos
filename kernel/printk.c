#include <stdarg.h>

#include <kernel/kernel.h>
#include <kernel/tty.h>
#include <libc/ctype.h>
#include <libc/string.h>

static int write_signed(char *dest, intmax_t v, int space, int plus)
{
	int val = v % 10;
	int nwritten = 0;

	if ((v / 10) != 0)
		nwritten = write_signed(dest, v / 10, space, plus);

	if (v < 0)
		val = -val;
	dest[nwritten++] = '0' + val;

	return nwritten;
}

static int write(const char *str, size_t len, int (*put)(int c, void*), void* opaque)
{
	int nwritten = 0;

	for (size_t i = 0; i < len; ++i) {
		int ret = put(str[i], opaque);
		if (ret < 0)
			return ret;
		nwritten += 1;
	}
	return nwritten;
}

static int pad(int ch, size_t n, int (*put)(int c, void*), void *opaque)
{
	int nwritten = 0;

	while (n--) {
		int ret = put(ch, opaque);
		if (ret < 0)
			return -1;
		nwritten += ret;
	}
	return nwritten;
}

static int vprintx(const char *fmt, va_list ap, int (*put)(int c, void*), void *opaque)
{
	int nwritten = 0;
	while (*fmt) {
		if (*fmt != '%') {
			int rc = put(*fmt++, opaque);
			if (rc < 0)
				goto write_error;

			nwritten += rc;
			continue;
		}

		fmt += 1;

		int hash = 0;
		int zero = 0;
		int minus = 0;
		int space = 0;
		int plus = 0;

		while (1) {
			int flag = *fmt;

			if (flag == '#')
				hash = 1;
			else if (flag == '0')
				zero = 1;
			else if (flag == '-')
				minus = 1;
			else if (flag == '+')
				plus = 1;
			else
				break;

			fmt += 1;
		}

		if (zero && minus)
			zero = 0;

		int width = -1;

		if (*fmt == '*') {
			width = va_arg(ap, int);

			if (width < 0) {
				minus = 1;
				width = -width;
			}

			fmt += 1;
		} else if (isdigit(*fmt)) {
			width = 0;
			while (isdigit(*fmt)) {
				width = width * 10 + (*fmt - '0');
				fmt += 1;
			}
		}

		int prec = -1;
		if (*fmt == '.') {
			fmt += 1;

			if (*fmt == '*') {
				prec = va_arg(ap, int);
			} else {

				if (*fmt == '-') {
					fmt += 1;

				} else  {
					prec = 0;

					while (isdigit(*fmt)) {
						prec = prec * 10 + (*fmt - '0');
						fmt += 1;
					}
				}
			}
		}

		char size = 0;

		if (strncmp(fmt, "hh", 2) == 0) {
			size = 'H';
			fmt += 1;
		} else if (strncmp(fmt, "h", 1) == 0) {
			size = 'H';
			fmt += 2;
		} else if (strncmp(fmt, "ll", 2) == 0) {
			size = 'L';
			fmt += 2;
		} else if (strncmp(fmt, "l", 1) == 0) {
			size = 'l';
			fmt += 1;
		}

		char buf[32];
		int padding = zero ? '0' : ' ';

		switch (*fmt++) {
			case 'd':
			case 'i':
				intmax_t v;

				if (size == 'L')
					v = va_arg(ap, long long);
				else if (size == 'l')
					v = va_arg(ap, long);
				else
					v = va_arg(ap, int);

				int numlen = write_signed(buf, v, space, plus);

				int totallen = numlen;
				if (v < 0 || plus || space)
					totallen += 1;
				if (prec > 0 && numlen < prec)
					totallen += prec - numlen;

				int ret = 0;
				if (width > 0 && !minus && totallen < width) {
					ret = pad(padding, width - totallen, put, opaque);
					if (ret < 0)
						goto write_error;
				}

				if (v < 0) {
					ret = put('-', opaque);
				} else if (plus) {
					ret = put('+', opaque);
				} else if (space) {
					ret = put(' ', opaque);
				}
				if (ret < 0)
					goto write_error;


				if (prec > 0 && numlen < prec) {
					ret = pad('0', prec - numlen, put, opaque);
					if (ret < 0)
						goto write_error;
				}

				ret = write(buf, numlen, put, opaque);
				if (ret < 0)
					goto write_error;


				if (width > 0 && minus && totallen < width) {
					ret = pad(padding, width - totallen, put, opaque);
					if (ret < 0)
						goto write_error;
				}

				break;
		}
	}

	return nwritten;
write_error:
	return -1;
}

static int write_to_term(int ch, void *)
{
	term_put(ch);
	return 1;
}

int printk(const char *fmt, ...)
{
	va_list args;
	va_start(args, fmt);

	int res = vprintx(fmt, args, write_to_term, NULL);

	va_end(args);
	return res;
}

#include <kernel/keycode.h>

int kc_toascii(enum keycode k)
{
	if (k >= KEYCODE_A && k <= KEYCODE_Z)
		return 'a' + (int) k - (int) KEYCODE_A;
	if (kc_isdigit(k))
		return '0' + kc_todigit(k);
	switch (k) {
	case KEYCODE_SPACE:
		return ' ';
	case KEYCODE_ENTER:
		return '\n';
	case KEYCODE_BACKSPACE:
		return '\b';
	default:
		return 0;
	}
}

bool kc_isdigit(enum keycode k)
{
	return k >= KEYCODE_0 && k <= KEYCODE_9;
}

int kc_todigit(enum keycode k)
{
	return k - (int) KEYCODE_0;
}

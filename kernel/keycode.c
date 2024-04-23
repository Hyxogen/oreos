#include <kernel/keycode.h>

int kc_toascii(enum keycode k)
{
	if (k >= KEYCODE_A && k <= KEYCODE_Z)
		return 'a' + (int) k - (int) KEYCODE_A;
	if (k >= KEYCODE_0 && k <= KEYCODE_9)
		return '0' + (int) k - (int) KEYCODE_0;
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

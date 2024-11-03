#include <kernel/signal.h>

bool is_valid_signal(int signum)
{
	return signum > 0 && signum < 32;
}

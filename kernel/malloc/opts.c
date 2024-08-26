#include <kernel/malloc/ma/internal.h>

#include <lib/stdlib.h>

static struct ma_opts *ma_get_opts_mut(void)
{
	static struct ma_opts opts;
	return &opts;
}

const struct ma_opts *ma_get_opts(void) { return ma_get_opts_mut(); }

void ma_init_opts(void)
{
	struct ma_opts *opts = ma_get_opts_mut();
	opts->perturb = false;
	opts->perturb_byte = 0x00;
}

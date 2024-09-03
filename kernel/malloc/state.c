#include <kernel/malloc/ma/internal.h>

#include <kernel/libc/stdlib.h>

// At the moment, the get_current_arena and get_arena functions don't do
// anything special, these are just placeholder functions for when I want to add
// proper multithreading support

// TODO properly initialize arena!
static struct ma_state state;

struct ma_arena *ma_get_current_arena(void) { return &state.main_arena; }

struct ma_arena *ma_get_arena(const void *p)
{
	(void)p;
	return &state.main_arena;
}

void ma_maybe_initialize(void)
{
	static bool ma_inited = false;
	if (!ma_inited) {
		state.initialized = true; //TODO remove from struct
		ma_inited = true;

		// we call ma_sysalloc_granularity here to load the pagesize and
		// avoid races when we actually need it during allocation
		ma_sysalloc_granularity();

		ma_init_opts();
		ma_init_arena(&state.main_arena);
	}
}

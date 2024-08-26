#include <kernel/malloc/ma/internal.h>

#include <lib/string.h>

static size_t ma_set_bit(size_t string, size_t mask, bool v)
{
	return (string & (~mask)) | mask * v;
}

void ma_set_size(struct ma_hdr *chunk, size_t newsize)
{
	ma_assert(!(newsize & MA_FLAGS_MASK));

	chunk->tag = (chunk->tag & MA_FLAGS_MASK) | newsize;
}

size_t ma_get_size(const void *tag)
{
	uint64_t *hdr = (uint64_t *)tag;
	return (size_t) *hdr & MA_SIZE_MASK;
}

void ma_set_pinuse(struct ma_hdr *chunk, bool v)
{
	chunk->tag = ma_set_bit(chunk->tag, MA_PINUSE_FLAG, v);
}

bool ma_is_pinuse(const struct ma_hdr *chunk)
{
	return chunk->tag & MA_PINUSE_FLAG;
}

void ma_set_small(struct ma_hdr *chunk, bool v)
{
	chunk->tag = ma_set_bit(chunk->tag, MA_SMALL_FLAG, v);
}

bool ma_is_small(const struct ma_hdr *chunk)
{
	return chunk->tag & MA_SMALL_FLAG;
}

void ma_set_large(struct ma_hdr *chunk, bool v)
{
	chunk->tag = ma_set_bit(chunk->tag, MA_LARGE_FLAG, v);
}

bool ma_is_large(const struct ma_hdr *chunk)
{
	return chunk->tag & MA_LARGE_FLAG;
}

bool ma_is_huge(const void *chunk)
{
	return !ma_is_small(chunk) && !ma_is_large(chunk);
}

bool ma_is_sentinel(const struct ma_hdr *chunk)
{
	return ma_get_size(chunk) == 0;
}

enum ma_size_class ma_get_size_class(const struct ma_hdr *hdr)
{
	ma_assert(!(ma_is_small(hdr) && ma_is_large(hdr)) &&
		  "corrupted or invalid chunk");
	if (ma_is_small(hdr))
		return MA_SMALL;
	if (ma_is_large(hdr))
		return MA_LARGE;
	ma_assert(ma_is_huge(hdr));
	return MA_HUGE;
}

struct ma_hdr *ma_next_hdr(const void *chunk)
{
	struct ma_hdr *hdr = (struct ma_hdr *)chunk;
	return (struct ma_hdr *)((unsigned char *)chunk + ma_get_size(hdr) +
				 MA_HEADER_SIZE);
}

struct ma_hdr *ma_prev_hdr(const void *chunk)
{
#ifndef FT_NDEBUG
	struct ma_hdr *hdr = (struct ma_hdr *)chunk;
	ma_assert(!ma_is_pinuse(hdr));
#endif

	size_t prev_size = ma_get_size((unsigned char *)chunk - MA_FOOTER_SIZE);
	return (struct ma_hdr *)((unsigned char *)chunk - prev_size -
				 MA_HEADER_SIZE);
}

static size_t *ma_get_ftr(const struct ma_hdr *chunk)
{
	return (size_t *)((unsigned char *)chunk + ma_get_size(chunk) +
			  MA_HEADER_SIZE - MA_FOOTER_SIZE);
}

static void ma_set_ftr(struct ma_hdr *chunk)
{
	*ma_get_ftr(chunk) = chunk->tag;
}

bool ma_is_inuse(const struct ma_hdr *hdr)
{
	ma_assert(!ma_is_sentinel(hdr));
	return ma_is_pinuse(ma_next_hdr(hdr));
}

void ma_set_inuse(struct ma_hdr *chunk, bool v)
{
	ma_set_pinuse(ma_next_hdr(chunk), v);
	if (!v)
		ma_set_ftr(chunk);
}

void *ma_chunk_to_mem(const struct ma_hdr *chunk)
{
	return (unsigned char *)chunk + MA_HEADER_SIZE;
}

struct ma_hdr *ma_mem_to_chunk(const void *p)
{
	return (struct ma_hdr *)((unsigned char *)p - MA_HEADER_SIZE);
}

void ma_make_sentinel(struct ma_hdr *chunk, enum ma_size_class class,
		      bool pinuse)
{
	ma_set_pinuse(chunk, pinuse);
	ma_set_small(chunk, class == MA_SMALL);
	ma_set_large(chunk, class == MA_LARGE);
	ma_set_size(chunk, 0);
}

struct ma_hdr *ma_init_chunk(struct ma_hdr *chunk, enum ma_size_class class,
			     size_t size, bool pinuse)
{
	ma_assert(size >= MA_MIN_ALLOC_SIZE);

	ma_set_pinuse(chunk, pinuse);
	ma_set_small(chunk, class == MA_SMALL);
	ma_set_large(chunk, class == MA_LARGE);
	ma_set_size(chunk, size);

	ma_set_ftr(chunk);

	struct ma_hdr *next = ma_next_hdr(chunk);
	ma_set_pinuse(next, false);
	return next;
}

struct ma_hdr *ma_find_bestfit(const struct ma_hdr *list, size_t size)
{
	const struct ma_hdr *cur = list;
	const struct ma_hdr *best = NULL;

	if (!cur)
		return NULL;

	do {
		ma_assert(!ma_is_inuse(cur));

		size_t chunk_size = ma_get_size(cur);

		if (chunk_size == size) {
			best = cur;
			break;
		} else if (chunk_size > size) {
			if (!best || chunk_size < ma_get_size(best))
				best = cur;
		}

		ma_assert(cur->next);
		cur = cur->next;
	} while (cur != list);

	return (struct ma_hdr *)best;
}

struct ma_hdr *ma_merge_chunks(struct ma_hdr *a, struct ma_hdr *b)
{
	ma_assert(a != b);
#if MA_SEGREGATED_BESTFIT
	ma_assert(ma_get_size_class(a) == ma_get_size_class(b));
#endif

	struct ma_hdr *first = a < b ? a : b;

	size_t new_size = ma_get_size(a) + ma_get_size(b) + MA_HEADER_SIZE;

	// TODO instead of ma_pinuse as last argument, true would probably also
	// work
	ma_init_chunk(first, ma_get_size_class(a), new_size,
		      ma_is_pinuse(first));
	return first;
}

struct ma_hdr *ma_split_chunk(struct ma_hdr *chunk, size_t newsize)
{
	ma_assert(ma_get_size(chunk) >= newsize);
	size_t rem = ma_get_size(chunk) - newsize;

	ma_assert(rem >= MA_MIN_CHUNK_SIZE);
	ma_assert(!ma_is_inuse(chunk));

	// TODO in theory only the size in the header and the footer have to be
	// reset, the _small, _large and _pinuse flags don't have to be set (for
	// chunk) and might even be (a bit faster).
	enum ma_size_class class = ma_get_size_class(chunk);
	struct ma_hdr *next = ma_init_chunk(chunk, class, newsize, true);

	ma_init_chunk(next, class, rem - MA_HEADER_SIZE, false);
	return next;
}

bool ma_should_split(const struct ma_hdr *chunk, size_t alloc_size)
{
#if MA_SEGREGATED_BESTFIT
	switch (ma_get_size_class(chunk)) {
	case MA_SMALL:
		return ma_get_size(chunk) - alloc_size >=
		       MA_MIN_SMALL_SIZE + MA_HEADER_SIZE;
	case MA_LARGE:
		return ma_get_size(chunk) - alloc_size >=
		       MA_MIN_LARGE_SIZE + MA_HEADER_SIZE;
	default:
		return false;
	}
#else
	return ma_get_size(chunk) - alloc_size >= MA_MIN_ALLOC_SIZE;
#endif
}

void ma_maybe_split(struct ma_arena *arena, struct ma_hdr *chunk,
		    size_t alloc_size)
{
	if (ma_should_split(chunk, alloc_size)) {
		struct ma_hdr *rem = ma_split_chunk(chunk, alloc_size);
		ma_append_chunk_any(arena, rem);
	}
}

void ma_append_chunk(struct ma_hdr **list, struct ma_hdr *chunk)
{
	ma_assert(!ma_is_inuse(chunk));

	if (!*list) {
		*list = chunk;
		chunk->next = chunk->prev = chunk;
		return;
	}
	chunk->prev = (*list)->prev;
	chunk->next = *list;

	(*list)->prev->next = chunk;
	(*list)->prev = chunk;
}

void ma_unlink_chunk(struct ma_hdr **list, struct ma_hdr *chunk)
{
	ma_assert(*list);

	if (chunk->next != chunk) {
		struct ma_hdr *next = chunk->next;
		struct ma_hdr *prev = chunk->prev;

		if (next == prev) {
			next->next = next->prev = next;
		} else {
			next->prev = prev;
			prev->next = next;
		}

		if (chunk == *list)
			*list = next;
	} else {
		// only element in list
		*list = NULL;
	}
}

struct ma_hdr *ma_alloc_chunk(struct ma_arena *arena, size_t minsize,
			      enum ma_size_class class)
{
	size_t actual_size = MA_ALIGN_UP(minsize + MA_CHUNK_ALLOC_PADDING,
					 ma_sysalloc_granularity());
	struct ma_hdr *chunk = ma_sysalloc(actual_size);

	if (chunk == MA_SYSALLOC_FAILED)
		return NULL;

	// See comment in top of ma/internal.h for explanation
	chunk = (struct ma_hdr *)((uintptr_t)chunk | MA_HALF_MALLOC_ALIGN);

	ma_debug_add_chunk(&arena->debug, chunk);

	size_t size = actual_size - MA_CHUNK_ALLOC_PADDING;
	struct ma_hdr *sentinel = ma_init_chunk(chunk, class, size, true);

	ma_make_sentinel(sentinel, class, false);

	return chunk;
}

void ma_dealloc_chunk(struct ma_arena *arena, struct ma_hdr *chunk)
{
	ma_assert(ma_is_sentinel(ma_next_hdr(chunk)) &&
		  "partially deallocate should not happen");
	void *start = (void *)MA_ALIGN_DOWN(chunk, ma_sysalloc_granularity());

	if (!ma_sysfree(start, ma_get_size(chunk) + MA_CHUNK_ALLOC_PADDING)) {
		ft_perror("ma_sysfree");
		ft_abort();
	}
	ma_debug_rem_chunk(&arena->debug, chunk);
}

void ma_chunk_fill(struct ma_hdr *hdr, uint8_t byte)
{
	memset(ma_chunk_to_mem(hdr), byte, ma_get_size(hdr));
}

bool ma_is_user_chunk(const struct ma_hdr *chunk)
{
	if (ma_is_sentinel(chunk))
		return false;
	if (ma_is_large(chunk) && ma_is_small(chunk))
		return false;

#if MA_SEGREGATED_BESTFIT
	size_t size = ma_get_size(chunk);
	if (ma_is_small(chunk))
		return size >= MA_MIN_SMALL_SIZE &&
		       size < MA_MAX_SMALL_SIZE + MA_MIN_SMALL_SIZE;
	if (ma_is_large(chunk))
		return size >= MA_MIN_LARGE_SIZE &&
		       size < MA_MAX_LARGE_SIZE + MA_MIN_LARGE_SIZE;
	return size >= MA_MAX_LARGE_SIZE;
#else
	// if we don't use segregated bestfit then we support aligned_alloc,
	// which might trim the chunk in a way that is not (usually) appropriate
	// for the size classes
	return true;
#endif
}

void ma_check_user_chunk(const struct ma_hdr *hdr)
{
	if (!ma_is_user_chunk(hdr)) {
		eprint("%p: invalid chunk\n", (void *)hdr);
		ft_abort();
	}
}

void ma_dump_chunk(const struct ma_hdr *chunk)
{
	if (ma_is_sentinel(chunk)) {
		eprint("%p SENTINEL\n", (void *)chunk);
		return;
	}

	bool inuse = ma_is_inuse(chunk);

	if (inuse)
		eprint("\033[31m");
	else
		eprint("\033[32m");

	size_t size = ma_get_size(chunk);

	const void *userptr = ma_chunk_to_mem(chunk);

	eprint("%p: %p - %p: ", (void *)chunk, (void *)userptr,
	       (void *)((unsigned char *)userptr + size));

	eprint("p=%i s=%i l=%i size=%7zu", ma_is_pinuse(chunk),
	       ma_is_small(chunk), ma_is_large(chunk), size);

	if (!inuse && !ma_is_huge(chunk)) {
		if (ma_is_binable(chunk))
			eprint(" bin=%3zu", ma_binidx(size));

		eprint(" next=%p prev=%p", (void *)chunk->next,
		       (void *)chunk->prev);
	}
	eprint("\033[m\n");
}

void ma_dump_all_chunks(const struct ma_hdr *list, void *unused)
{
	(void)unused;
	const struct ma_hdr *cur = list;
	if (!cur)
		return;

	eprint("%p:\n", (void *)list);
	while (1) {
		ma_dump_chunk(cur);

		if (ma_is_sentinel(cur))
			break;
		cur = ma_next_hdr(cur);
	}
}

#if !defined(FT_NDEBUG) && MA_CHECK_SELF
void ma_assert_correct_chunk(const struct ma_hdr *chunk)
{
	ma_assert((uintptr_t)chunk & MA_HALF_MALLOC_ALIGN);

	if (ma_is_sentinel(chunk))
		return;

	ma_assert(!(ma_is_small(chunk) && ma_is_large(chunk)));

	struct ma_hdr *next = ma_next_hdr(chunk);

	if (ma_is_inuse(chunk)) {
		ma_assert(ma_is_pinuse(next));
	} else {
		size_t ftr = *ma_get_ftr(chunk);
		size_t hdr = *(size_t *)chunk;

		ma_assert((ftr & ~MA_PINUSE_FLAG) == (hdr & ~MA_PINUSE_FLAG));

		if (!ma_is_huge(chunk)) {
			ma_assert(chunk->next);
			ma_assert(chunk->prev);

			ma_assert(chunk->next->prev == chunk);
			ma_assert(chunk->prev->next == chunk);
		}

		ma_assert(ma_is_pinuse(chunk) && "chunks should be merged");

		ma_assert(ma_is_pinuse(next) == false);

		if (!ma_is_sentinel(next))
			ma_assert(ma_is_inuse(next) &&
				  "chunks should be merged");
	}
}

void ma_assert_correct_all_chunks(const struct ma_hdr *list, void *unused)
{
	(void)unused;
	const struct ma_hdr *cur = list;
	if (!cur)
		return;

	while (1) {
		ma_assert_correct_chunk(cur);

		if (ma_is_sentinel(cur))
			break;
		cur = ma_next_hdr(cur);
	}
}
#endif

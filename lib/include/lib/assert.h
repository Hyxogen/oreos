#ifndef __LIB_ASSERT_H
#define __LIB_ASSERT_H

#ifndef NDEBUG
void __assert_impl(int c, const char *pred, const char *file, const char *func,
		   int line);
#define assert(c) __assert_impl((c) != 0, #c, __FILE__, __FUNCTION__, __LINE__)
#else
#define assert(c)
#endif

#endif

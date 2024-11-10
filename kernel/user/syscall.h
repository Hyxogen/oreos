#ifndef __KERNEL_USER_SYSCALL_H
#define __KERNEL_USER_SYSCALL_H

#define __SYSCAST(x) ((long) (x))

#define errno __errno

extern long __errno;

long __syscall0(long a);
long __syscall1(long a, long b);
long __syscall2(long a, long b, long c);
long __syscall3(long a, long b, long c, long d);
long __syscall4(long a, long b, long c, long d, long e);
long __syscall5(long a, long b, long c, long d, long e, long f);
long __syscall6(long a, long b, long c, long d, long e, long f, long g);

long __syscall_ret_wrapper(long res);

#define __SYSCALL0(num) __syscall0(num)
#define __SYSCALL1(num, a) __syscall1(num, __SYSCAST(a))
#define __SYSCALL2(num, a, b) __syscall2(num, __SYSCAST(a), __SYSCAST(b))
#define __SYSCALL3(num, a, b, c) \
	__syscall3(num, __SYSCAST(a), __SYSCAST(b), __SYSCAST(c))
#define __SYSCALL4(num, a, b, c, d) \
	__syscall4(num, __SYSCAST(a), __SYSCAST(b), __SYSCAST(c), __SYSCAST(d))
#define __SYSCALL5(num, a, b, c, d, e)                            \
	__syscall5(num, __SYSCAST(a), __SYSCAST(b), __SYSCAST(c), \
		   __SYSCAST(d), __SYSCAST(e))
#define __SYSCALL6(num, a, b, c, d, e, f)                         \
	__syscall6(num, __SYSCAST(a), __SYSCAST(b), __SYSCAST(c), \
		   __SYSCAST(d), __SYSCAST(e), __SYSCAST(f))

#define __SYSCALL_NARGS_IMPL(a, b, c, d, e, f, g, h, n, ...) n
#define __SYSCALL_NARGS(...) __SYSCALL_NARGS_IMPL(__VA_ARGS__, 7, 6, 5, 4, 3, 2, 1, 0,)
#define __SYSCALL_CONCAT_IMPL(a, b) a##b
#define __SYSCALL_CONCAT(a, b) __SYSCALL_CONCAT_IMPL(a, b)
#define __syscall(...) __SYSCALL_CONCAT(__SYSCALL, __SYSCALL_NARGS(__VA_ARGS__))(__VA_ARGS__)
#define syscall(...) __syscall_ret_wrapper(__syscall(__VA_ARGS__))

#endif

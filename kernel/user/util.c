#include "util.h"
#include "syscall.h"
#include "syscall_nr.h"

long __errno = 0;
void (*__signal_handlers[32])(int);

void SIG_DFL(int signum)
{
	switch (signum) {
	case SIGALRM:
	case SIGCHLD:
		return;
	case SIGSEGV:
	default:
		_exit(-signum);
	}
}

static int __signal(int sig, void (*handler)(int))
{
	return syscall(SYS_signal, sig, handler);
}

void __init(void)
{
	for (unsigned i = 0; i < sizeof(__signal_handlers)/sizeof(__signal_handlers[0]); i++) {
		__signal_handlers[i] = SIG_DFL;
		__signal(i, __signal_trampoline);
	}
}

int wait(int *wstatus)
{
	return waitpid(-1, wstatus, 0);
}

unsigned int sleep(unsigned int seconds)
{
	alarm(seconds);
	pause();
	return alarm(0);
}

ssize_t write(int fd, const void *src, size_t nbytes)
{
	return syscall(SYS_write, fd, src, nbytes);
}

ssize_t read(int fd, void *dest, size_t nbytes)
{
	return syscall(SYS_read, fd, dest, nbytes);
}

int fork(void)
{
	return syscall(SYS_fork);
}

void _exit(int exit_code)
{
	while (1) {
		syscall(SYS_exit, exit_code);
	}
}

int close(int fd)
{
	return syscall(SYS_close, fd);
}

pid_t getpid(void)
{
	return syscall(SYS_getpid);
}

int kill(pid_t pid, int sig)
{
	return syscall(SYS_kill, pid, sig);
}

int waitpid(pid_t pid, int *wstatus, int options)
{
	return syscall(SYS_waitpid, pid, wstatus, options);
}

int alarm(unsigned int seconds)
{
	return syscall(SYS_alarm, seconds);
}

int pause(void)
{
	return syscall(SYS_pause);
}

int socketpair(int domain, int type, int protocol, int sv[2])
{
	return syscall(SYS_socketpair, domain, type, protocol, sv);
}

void* signal(int signum, void (*handler)(int))
{
	void *prev = (void*) __signal_handlers[signum];
	__signal_handlers[signum] = handler;
	return prev;
}

long __syscall_ret_wrapper(long res)
{
	if (res < 0) {
		errno = -res;
		return -1;
	}
	return res;
}

void exit(int exit_code)
{
	/* TODO this should call the on_exit hooks */
	_exit(exit_code);
}

uid_t getuid(void)
{
	return syscall(SYS_getuid);
}

size_t strlen(const char *str)
{
	const char *tmp = str;

	while (*tmp)
		++tmp;
	return tmp - str;
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

int isdigit(int c)
{
	return (unsigned)c - '0' < 10;
}

void *mmap(void *addr, size_t len, int prot, int flags, int fd, off_t offset)
{
	return (void*) syscall(SYS_mmap, addr, len, prot, flags, fd, offset);
}

int munmap(void *addr, size_t len)
{
	return syscall(SYS_munmap, addr, len);
}

void *memset(void *dest, int c, size_t n)
{
	unsigned char *d = dest;

	while (n--)
		*d++ = (unsigned char)c;
	return dest;
}

#ifndef __KERNEL_USER_H
#define __KERNEL_USER_H

#include <stddef.h>
#include <stdarg.h>

#define SIGKILL 9
#define SIGSEGV 11
#define SIGALRM 14
#define SIGCHLD 17
#define SIGSTOP 19

#define STDOUT_FILENO 0

#define MAP_PROT_READ 0x1
#define MAP_PROT_WRITE 0x2

#define MAP_PRIVATE 0x01
#define MAP_ANONYMOUS 0x02

typedef long ssize_t;
typedef long uid_t;
typedef int pid_t;
typedef long off_t;

void __signal_trampoline(int signum);

ssize_t write(int fd, const void *src, size_t nbytes);
ssize_t read(int fd, void *dest, size_t nbytes);
int fork(void);
__attribute__((noreturn))
void _exit(int exit_code);
int close(int fd);
pid_t getpid(void);
int kill(pid_t pid, int sig);
int waitpid(pid_t pid, int *wstatus, int options);
int alarm(unsigned int seconds);
int pause(void);
int socketpair(int domain, int type, int protocol, int sv[2]);
int wait(int *wstatus);
unsigned int sleep(unsigned int seconds);
void* signal(int signum, void (*handler)(int));
uid_t getuid(void);
void *mmap(void *addr, size_t len, int prot, int flags, int fd, off_t offset);
int munmap(void *addr, size_t len);
int sched_yield(void);

__attribute__((noreturn))
void exit(int exit_code);

void __init();

size_t strlen(const char *str);
int strncmp(const char *s1, const char *s2, size_t n);
int isdigit(int c);
void *memset(void *dest, int c, size_t n);

int snprintf(char *str, size_t size, const char *fmt, ...);
int vdprintf(int fd, const char *fmt, va_list ap);
int dprintf(int fd, const char *fmt, ...);
int vprintf(const char *fmt, va_list ap);
int printf(const char *fmt, ...);

#endif

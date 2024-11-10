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

typedef long ssize_t;
typedef long uid_t;
typedef int pid_t;

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

__attribute__((noreturn))
void exit(int exit_code);

void __init();

size_t strlen(const char *str);
int strncmp(const char *s1, const char *s2, size_t n);
int isdigit(int c);

int snprintf(char *str, size_t size, const char *fmt, ...);
int vdprintf(int fd, const char *fmt, va_list ap);
int dprintf(int fd, const char *fmt, ...);
int vprintf(const char *fmt, va_list ap);
int printf(const char *fmt, ...);

#endif

#include <stddef.h>

int write(int fd, const char *src, size_t nbytes);
int read(int fd, char *dest, size_t nbytes);
__attribute__((noreturn))
void exit(int exit_code);
int fork(void);
int __signal(int signum, void (*handler)(int));
int getpid(void);
int kill(int pid, int sig);
void __signal_trampoline(int signum);

void (*__signal_handlers[32])(int);

static void init(void)
{
	for (unsigned i = 0; i < sizeof(__signal_handlers)/sizeof(__signal_handlers[0]); i++) {
		__signal(i, __signal_trampoline);
	}
}

void* signal(int signum, void (*handler)(int))
{
	void *prev = (void*) __signal_handlers[signum];
	__signal_handlers[signum] = handler;
	return prev;
}

static void handler(int signum)
{
	(void)signum;
	write(0, "x", 1);
}

void _start(void)
{
	init();

	char buf = 'x';
	write(0, "hello world!\n", 13); 

	signal(10, handler);

	int pid = getpid();
	kill(pid, 10);

	while (1) {
		if (read(0, &buf, 1) != 1)
			continue;

		int res = fork();
		if (res < 0) {
			exit(-3);
		} else if (res == 0) {
			write(0, &buf, 1);
			exit(0);
		}
	}
}

#include <stddef.h>

int write(int fd, const char *src, size_t nbytes);
int read(int fd, char *dest, size_t nbytes);
__attribute__((noreturn))
void exit(int exit_code);
int fork(void);
int signal(int signum, void (*handler)(int));

static void handler(int signum)
{
	(void)signum;
	exit(42);
}

void _start(void)
{
	char buf = 'x';
	write(0, "hello world!\n", 13); 
	signal(11, handler);
	*(volatile int *)0 = 12;
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

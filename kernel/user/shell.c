#include <stddef.h>

#define SIGKILL 9
#define SIGSEGV 11
#define SIGALRM 14
#define SIGCHLD 17
#define SIGSTOP 19

int write(int fd, const char *src, size_t nbytes);
int read(int fd, char *dest, size_t nbytes);
int waitpid(int pid, int *wstatus, int options);
__attribute__((noreturn))
void exit(int exit_code);
int fork(void);
int __signal(int signum, void (*handler)(int));
int getpid(void);
int kill(int pid, int sig);
void __signal_trampoline(int signum);
unsigned int alarm(unsigned int seconds);
int pause(void);
int socketpair(int domain, int type, int protocol, int sv[2]);

void (*__signal_handlers[32])(int);

long syscall(long number, ...); /* TODO implement */

void SIG_DFL(int signum)
{
	switch (signum) {
	case SIGALRM:
	case SIGCHLD:
		return;
	case SIGSEGV:
	default:
		exit(-signum);
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

static void init(void)
{
	for (unsigned i = 0; i < sizeof(__signal_handlers)/sizeof(__signal_handlers[0]); i++) {
		__signal_handlers[i] = SIG_DFL;
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

size_t strlen(const char *str)
{
	const char *tmp = str;

	while (*tmp)
		++tmp;
	return tmp - str;
}

void writestrto(int fd, const char *str)
{
	size_t len = strlen(str);

	write(fd, str, len);
}

void writestr(const char *str)
{
	writestrto(0, str);
}

static void seghandler(int signum)
{
	(void) signum;
	if (__signal_handlers[11] == SIG_DFL) {
		writestr("yes");
	} else {
		writestr("no");
	}
}

static void test_socket(void)
{
	int sv[2];
	socketpair(1, 1, 0, sv);

	int res = fork();

	char buf[64];

	if (res == 0) {
		int nread = read(sv[1], buf, sizeof(buf));
		if (nread < 0) {
			writestr("child: failed to read\n");
		} else if (nread == 0) {
			writestr("child: socket was closed\n");
		} else {
			writestr("child: from parent: ");
			write(0, buf, nread);
			writestr("\n");
		}

		writestrto(sv[1], "hello world form child");
		exit(0);
	} else if (res > 0) {
		writestrto(sv[0], "hello world from parent\n");
		int nread = read(sv[0], buf, sizeof(buf));
		if (nread < 0) {
			writestr("parent: failed to read\n");
		} else if (nread == 0) {
			writestr("parent: socket was closed\n");
		} else {
			writestr("parent: from child: ");
			write(0, buf, nread);
			writestr("\n");
		}

	} else {
		writestr("failed to fork");
	}
}

void _start(void)
{
	init();

	test_socket();

	char buf = 'x';
	write(0, "hello world!\n", 13); 

	signal(10, handler);

	int pid = getpid();
	kill(pid, 10);

	signal(11, seghandler);
	kill(pid, 11);
	if (__signal_handlers[11] == seghandler) {
		writestr("yes2\n");
	} else {
		writestr("no2\n");
	}

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

		res = wait(NULL);
		if (!res) {
			writestr("a child exited\n");
		} else {
			writestr("no children\n");
		}
	}
}

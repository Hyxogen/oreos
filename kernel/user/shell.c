#include <stddef.h>
#include "util.h"

static void handler(int signum)
{
	(void)signum;
	write(0, "x", 1);
}

static void writestr(int fd, const char *str)
{
	write(fd, str, strlen(str));
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
			printf("child: failed to read\n");
		} else if (nread == 0) {
			printf("child: socket was closed\n");
		} else {
			printf("child: from parent: ");
			write(0, buf, nread);
			printf("\n");
		}

		writestr(sv[1], "hello world from child");
		exit(0);
	} else if (res > 0) {
		writestr(sv[0], "hello world from parent\n");
		int nread = read(sv[0], buf, sizeof(buf));
		if (nread < 0) {
			printf("parent: failed to read\n");
		} else if (nread == 0) {
			printf("parent: socket was closed\n");
		} else {
			printf("parent: from child: ");
			write(0, buf, nread);
			printf("\n");
		}

	} else {
		printf("failed to fork");
	}
}

void _start(void)
{
	__init();

	test_socket();

	char buf = 'x';
	printf("hello world!\n");

	signal(10, handler);

	int pid = getpid();
	kill(pid, 10);
	
	void *tmp = mmap(NULL, 4096, MAP_PROT_WRITE | MAP_PROT_READ, MAP_PRIVATE | MAP_ANONYMOUS, 0, 0);
	if (tmp == 0) {
		printf("failed to map memory\n");
	} else {
		printf("got some memory %p\n", tmp);
	}
	/* TODO munmap */

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
			printf("a child exited\n");
		} else {
			printf("no children\n");
		}
	}
}

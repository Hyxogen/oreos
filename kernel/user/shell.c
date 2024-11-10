#include <stddef.h>
#include "util.h"

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
	__init();

	test_socket();

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

		res = wait(NULL);
		if (!res) {
			writestr("a child exited\n");
		} else {
			writestr("no children\n");
		}
	}
}

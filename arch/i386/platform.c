#include <kernel/kernel.h>
#include <kernel/tty.h> //TTY will always be available
#include <kernel/ps2.h>

__attribute__((noreturn)) void _idle();

void reset(void)
{
	while (!ps2_cansend())
		continue;
	ps2_send_cmd(PS2_CMD_RESET);
	_idle();
}

void halt(void)
{
	_idle();
}

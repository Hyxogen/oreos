#include <stdatomic.h>
#include <kernel/ps2.h>
#include <kernel/kernel.h>
#include <kernel/printk.h>
#include <kernel/platform.h>
#include <kernel/irq.h>
#include <kernel/sync.h>
#include <kernel/libc/assert.h>

static u8 ps2_dev_type1, ps2_dev_type2;

/* TODO 30 tries is not enough for VirtualBox, increase the number or write
 * proper interrupt driver */
#define PS2_TIMEOUT_TRIES 30

#define PS2_BUFSIZE 128
enum keycode ps2_tokeycode_qwerty(u8 b);
enum keycode ps2_tokeycode_azerty(u8 b);

static atomic_bool _ps2_has_data = false;
static u8 _ps2_buf[PS2_BUFSIZE];
static u8 _ps2_write = 0, _ps2_read = 0;
static struct mutex _ps2_mutex;
static struct condvar _ps2_not_empty_cond;
static enum keycode (*_ps2_tokeycode)(u8) = ps2_tokeycode_qwerty;

static enum irq_result ps2_on_event(u8 irq, struct cpu_state *state, void *dummy)
{
	(void) irq;
	(void) dummy;
	(void) state;
	//TODO Possible race when trying to read buffer during this interrrupt
	//TODO instead of masking all interrupts, making a custom mask to
	//disable only this interrupt?

	// this interrupts handler should not be called again until ps2_eoi()
	_ps2_buf[_ps2_write] = ps2_recv();

	_ps2_write = (_ps2_write + 1) % PS2_BUFSIZE;
	if (_ps2_write == _ps2_read)
		_ps2_read += 1;

	atomic_store(&_ps2_has_data, true);

	condvar_signal(&_ps2_not_empty_cond); /* condvars are not irq safe, so must be before ps2_eoi() */
	ps2_eoi();
	return IRQ_CONTINUE;
}

static void ps2_clear(void)
{
	disable_irqs();
	_ps2_read = _ps2_write = 0;
	enable_irqs();
}

static int _ps2_get(void)
{
	int res = -1;
	if (_ps2_write != _ps2_read) {
		res = _ps2_buf[_ps2_read];

		_ps2_read = (_ps2_read + 1) % PS2_BUFSIZE;
	}
	atomic_store(&_ps2_has_data, _ps2_write != _ps2_read);
	return res;
}

static int ps2_get(void)
{
	int res = -1;

	if (atomic_load(&_ps2_has_data)) {
		disable_irqs();
		res = _ps2_get();
		enable_irqs();
	}
	return res;
}

static u8 ps2_get_wait(void)
{
	int res;

	while ((res = ps2_get()) < 0)
		short_wait();
	return res;
}

size_t ps2_read(void *dest, size_t n)
{
	char *c = dest;

	/* TODO use semaphore */
	mutex_lock(&_ps2_mutex);

	if (!atomic_load(&_ps2_has_data))
		condvar_wait(&_ps2_not_empty_cond, &_ps2_mutex);

	size_t nread = 0;
	while (nread < n) {
		int ch = ps2_get();

		if (ch == -1)
			break;
		enum keycode kc = _ps2_tokeycode((u8)ch);

		int val = kc_toascii(kc);
		if (val == 0)
			continue;

		/* TODO this should not be here, this driver should also be able
		 * to be used from the kernel */
		int res  = put_user1(c, (u8) val);
		if (res) {
			nread = 0;
			break;
		}

		c++;
		nread++;
	}

	mutex_unlock(&_ps2_mutex);
	return nread;
}

static int ps2_recv_timeout(void)
{
	for (int i = 0; i < PS2_TIMEOUT_TRIES; ++i) {
		if (ps2_canrecv())
			return ps2_recv();
		short_wait();
	}
	return -1;
}

static int ps2_send_timeout(u8 b)
{
	for (int i = 0; i < PS2_TIMEOUT_TRIES; ++i) {
		if (ps2_cansend()) {
			ps2_send(b);
			return 0;
		}
		short_wait();
	}
	return -1;
}

static int ps2_send_cmd_timeout(u8 b)
{
	for (int i = 0; i < PS2_TIMEOUT_TRIES; ++i) {
		if (ps2_cansend()) {
			ps2_send_cmd(b);
			return 0;
		}
		short_wait();
	}
	return -1;
}

/* can only be used after init_ps2_controller */
static int ps2_send_ack(u8 b)
{
	for (int i = 0; i < 3; ++i) {
		if (ps2_send_timeout(b) < 0)
			return -1;

		for (int i = 0; i < 16; i++)
			short_wait();

		int res = ps2_get();
		if (res < 0)
			break;
		if (res == PS2_RESEND)
			continue;
		if (res == PS2_ACK)
			return res;
		return -(int)res;
	}
	return -1;
}

static enum keycode ps2_getkey(void)
{
	return _ps2_tokeycode(ps2_recv());
}

enum keycode ps2_getkey_timeout(void)
{
	for (int i = 0; i < 30; ++i) {
		if (ps2_canrecv()) {
			return ps2_getkey();
		}
	}
	return KEYCODE_NONE;
}

static bool ps2_disable_ports(void)
{
	if (ps2_send_cmd_timeout(PS2_CMD_DISABLE_PORT1)) {
		printk("failed to disable ps2 port1\n");
		return false;
	}

	if (ps2_send_cmd_timeout(PS2_CMD_DISABLE_PORT2)) {
		printk("failed to disable ps2 port2\n");
		return false;
	}

	//flush output buffer, we don't care what we got
	ps2_recv();
	return true;
}

static bool ps2_enable_ports(void)
{
	if (ps2_send_cmd_timeout(PS2_CMD_ENABLE_PORT1)) {
		printk("ps2: failed to enable port1\n");
		return false;
	}
	return true;
}

static i16 ps2_read_conf_byte(void)
{
	if (ps2_send_cmd_timeout(PS2_CMD_READ_CONF_BYTE)) {
		printk("failed to get ps2 config byte\n");
		return -1;
	}
	u8 conf = ps2_recv_timeout();
	return conf;
}

static bool ps2_write_conf_byte(u8 conf)
{
	if (ps2_send_cmd_timeout(PS2_CMD_WRITE_CONF_BYTE)) {
		printk("failed to write ps2 config byte (1)\n");
		return false;
	}

	if (ps2_send_timeout(conf) < 0) {
		printk("failed to write ps2 config byte (2)\n");
		return false;
	}
	return true;
}

static bool ps2_test(void)
{
	//apparently this might reset the controller, if you have issues with
	//the keyboard, it's probably a good idea to check here first
	if (ps2_send_cmd_timeout(PS2_CMD_TEST_CONTROLLER)) {
		printk("failed to test controller\n");
		return false;
	}
	u8 res = ps2_recv_timeout();

	if (res != PS2_CONTROLLER_OK) {
		printk("ps2 controller self test failed\n");
		return false;
	}

	if (ps2_send_cmd_timeout(PS2_CMD_TEST_PORT1)) {
		printk("ps2: failed to test port1\n");
		return false;
	}
	res = ps2_recv_timeout();

	if (res != PS2_PORT_OK) {
		printk("ps2 port1 self test failed\n");
		return false;
	}
	return true;
}

static void init_ps2_controller(void)
{
	//TODO check if controller exists once we are using ACPI
	//TODO also properly test port2
	
	if (!ps2_disable_ports())
		return;

	i16 conf = ps2_read_conf_byte();
	if (conf < 0)
		return;

	//disable interrupts
	conf &= ~(PS2_CONF_PORT1_IRQ | PS2_CONF_PORT2_IRQ);

	if (!ps2_write_conf_byte(conf))
		return;

	if (!ps2_test())
		return;

	// enable interrupts
	conf |= PS2_CONF_PORT1_IRQ;
	if (!ps2_write_conf_byte(conf))
		return;

	if (!ps2_enable_ports())
		return;

	if (ps2_send_timeout(PS2_RESET) < 0) {
		printk("failed to reset device on port1\n");
		return;
	}

	// we are now using interrupts, thus the read_timeout functions should
	// not be used anymore
	int b1 = ps2_get_wait();
	int b2 = ps2_get_wait();

	if ((b1 == PS2_ACK && b2 == PS2_PASSED_SELFTEST) || (b2 == PS2_ACK && b1 == PS2_PASSED_SELFTEST)) {
		printk("setup ps2 controller\n");
	} else {
		printk("failed to setup ps2 controller: %i, %i\n", b1, b2);
	}
}

static void ps2_identify(void)
{
	ps2_clear();
	if (ps2_send_ack(PS2_DISABLE_SCANNING) < 0) {
		printk("failed to disable scanning 1\n");
		return;
	}

	if (ps2_send_ack(PS2_IDENTIFY) < 0) {
		printk("failed to identify ps/2 device\n");
		return;
	}

	ps2_dev_type1 = ps2_dev_type2 = 0;

	int res = ps2_get();
	if (res > 0) {
		ps2_dev_type1 = (u8)res;

		res = ps2_get();
		if (res > 0) {
			ps2_dev_type2 = (u8)res;
		}
	}

	printk("identified keyboard: 0x%x 0x%x\n", ps2_dev_type1,
	       ps2_dev_type2);

	if (ps2_send_ack(PS2_ENABLE_SCANNING) < 0)
		printk("failed to re-enable scanning\n");
}

static void ps2_enable_irq(void)
{
	i16 res = ps2_setup_irq();
	if (res < 0) {
		printk("failed to enable irqs for ps2\n");
		return;
	}

	if (irq_register_handler((u8)res, ps2_on_event, NULL) < 0) {
		printk("failed to register ps2 irq handler\n");
		return;
	}
	printk("enabled ps2 interrupts\n");
}

void ps2_set_mode(int mode)
{
	switch (mode){
	case 0:
		_ps2_tokeycode = ps2_tokeycode_qwerty;
		break;
	case 1:
		_ps2_tokeycode = ps2_tokeycode_azerty;
		break;
	default:
		panic("don't use this stupid function");
	}
}

void init_ps2(void)
{
	ps2_enable_irq();
	init_ps2_controller();
	ps2_identify();
	mutex_init(&_ps2_mutex, 0);
	condvar_init(&_ps2_not_empty_cond);
}

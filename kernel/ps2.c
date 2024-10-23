#include <kernel/ps2.h>
#include <kernel/kernel.h>
#include <kernel/printk.h>
#include <kernel/platform.h>
#include <kernel/irq.h>

static u8 ps2_dev_type1, ps2_dev_type2;

/* TODO 30 tries is not enough for VirtualBox, increase the number or write
 * proper interrupt driver */
#define PS2_TIMEOUT_TRIES 30

#define PS2_BUFSIZE 128

static u8 _ps2_buf[PS2_BUFSIZE];
static u8 _ps2_write = 0, _ps2_read = 0;

static enum keycode ps2_tokeycode(u8 b)
{
	switch (b) {
	case PS2_SCANCODE_A_PRESSED:
		return KEYCODE_A;
	case PS2_SCANCODE_B_PRESSED:
		return KEYCODE_B;
	case PS2_SCANCODE_C_PRESSED:
		return KEYCODE_C;
	case PS2_SCANCODE_D_PRESSED:
		return KEYCODE_D;
	case PS2_SCANCODE_E_PRESSED:
		return KEYCODE_E;
	case PS2_SCANCODE_F_PRESSED:
		return KEYCODE_F;
	case PS2_SCANCODE_G_PRESSED:
		return KEYCODE_G;
	case PS2_SCANCODE_H_PRESSED:
		return KEYCODE_H;
	case PS2_SCANCODE_I_PRESSED:
		return KEYCODE_I;
	case PS2_SCANCODE_J_PRESSED:
		return KEYCODE_J;
	case PS2_SCANCODE_K_PRESSED:
		return KEYCODE_K;
	case PS2_SCANCODE_L_PRESSED:
		return KEYCODE_L;
	case PS2_SCANCODE_M_PRESSED:
		return KEYCODE_M;
	case PS2_SCANCODE_N_PRESSED:
		return KEYCODE_N;
	case PS2_SCANCODE_O_PRESSED:
		return KEYCODE_O;
	case PS2_SCANCODE_P_PRESSED:
		return KEYCODE_P;
	case PS2_SCANCODE_Q_PRESSED:
		return KEYCODE_Q;
	case PS2_SCANCODE_R_PRESSED:
		return KEYCODE_R;
	case PS2_SCANCODE_S_PRESSED:
		return KEYCODE_S;
	case PS2_SCANCODE_T_PRESSED:
		return KEYCODE_T;
	case PS2_SCANCODE_U_PRESSED:
		return KEYCODE_U;
	case PS2_SCANCODE_V_PRESSED:
		return KEYCODE_V;
	case PS2_SCANCODE_W_PRESSED:
		return KEYCODE_W;
	case PS2_SCANCODE_X_PRESSED:
		return KEYCODE_X;
	case PS2_SCANCODE_Z_PRESSED:
		return KEYCODE_Z;
	case PS2_SCANCODE_Y_PRESSED:
		return KEYCODE_Y;
	case PS2_SCANCODE_1_PRESSED:
		return KEYCODE_1;
	case PS2_SCANCODE_2_PRESSED:
		return KEYCODE_2;
	case PS2_SCANCODE_3_PRESSED:
		return KEYCODE_3;
	case PS2_SCANCODE_4_PRESSED:
		return KEYCODE_4;
	case PS2_SCANCODE_5_PRESSED:
		return KEYCODE_5;
	case PS2_SCANCODE_6_PRESSED:
		return KEYCODE_6;
	case PS2_SCANCODE_7_PRESSED:
		return KEYCODE_7;
	case PS2_SCANCODE_8_PRESSED:
		return KEYCODE_8;
	case PS2_SCANCODE_9_PRESSED:
		return KEYCODE_9;
	case PS2_SCANCODE_0_PRESSED:
		return KEYCODE_0;
	case PS2_SCANCODE_ENTER_PRESSED:
		return KEYCODE_ENTER;
	case PS2_SCANCODE_SPACE_PRESSED:
		return KEYCODE_SPACE;
	case PS2_SCANCODE_BACKSPACE_PRESSED:
		return KEYCODE_BACKSPACE;
	case PS2_SCANCODE_LCTRL_PRESSED:
		return KEYCODE_LCTRL;
	case PS2_SCANCODE_F1_PRESSED:
		return KEYCODE_F1;
	case PS2_SCANCODE_F2_PRESSED:
		return KEYCODE_F2;
	case PS2_SCANCODE_F3_PRESSED:
		return KEYCODE_F3;
	case PS2_SCANCODE_F4_PRESSED:
		return KEYCODE_F4;
	case PS2_SCANCODE_F5_PRESSED:
		return KEYCODE_F5;
	case PS2_SCANCODE_F6_PRESSED:
		return KEYCODE_F6;
	case PS2_SCANCODE_F7_PRESSED:
		return KEYCODE_F7;
	case PS2_SCANCODE_F8_PRESSED:
		return KEYCODE_F8;
	case PS2_SCANCODE_F9_PRESSED:
		return KEYCODE_F9;
	case PS2_SCANCODE_F10_PRESSED:
		return KEYCODE_F10;
	case PS2_SCANCODE_F11_PRESSED:
		return KEYCODE_F11;
	case PS2_SCANCODE_F12_PRESSED:
		return KEYCODE_F12;
	case PS2_SCANCODE_SEMI_PRESSED:
	case PS2_SCANCODE_SINGLE_QUOTE_PRESSED:
	case PS2_SCANCODE_BACK_TICK_PRESSED:
	case PS2_SCANCODE_LSHIFT_PRESSED:
	case PS2_SCANCODE_BACKSLASH_PRESSED:
	case PS2_SCANCODE_COMMA_PRESSED:
	case PS2_SCANCODE_PERIOD_PRESSED:
	case PS2_SCANCODE_FORWARDSLASH_PRESSED:
	case PS2_SCANCODE_RSHIFT_PRESSED:
	case PS2_SCANCODE_KP_STAR_PRESSED:
	case PS2_SCANCODE_LALT_PRESSED:
	case PS2_SCANCODE_CAPSLOCK_PRESSED:
	case PS2_SCANCODE_ESC_PRESSED:
	case PS2_SCANCODE_MINUS_PRESSED:
	case PS2_SCANCODE_EQUALS_PRESSED:
	case PS2_SCANCODE_TAB_PRESSED:
	case PS2_SCANCODE_LBRACK_PRESSED:
	case PS2_SCANCODE_RBRACK_PRESSED:
	case PS2_SCANCODE_NUMLOCK_PRESSED:
	case PS2_SCANCODE_SCROLLLOCK_PRESSED:
	case PS2_SCANCODE_KP_7_PRESSED:
	case PS2_SCANCODE_KP_8_PRESSED:
	case PS2_SCANCODE_KP_9_PRESSED:
	case PS2_SCANCODE_KP_MINUS_PRESSED:
	case PS2_SCANCODE_KP_4_PRESSED:
	case PS2_SCANCODE_KP_5_PRESSED:
	case PS2_SCANCODE_KP_6_PRESSED:
	case PS2_SCANCODE_KP_PLUS_PRESSED:
	case PS2_SCANCODE_KP_1_PRESSED:
	case PS2_SCANCODE_KP_2_PRESSED:
	case PS2_SCANCODE_KP_3_PRESSED:
	case PS2_SCANCODE_KP_0_PRESSED:
	case PS2_SCANCODE_KP_PERIOD_PRESSED:
	default:
		return KEYCODE_UNKNOWN;
	}
}


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
	return res;
}

static int ps2_get(void)
{
	disable_irqs();
	int res = _ps2_get();
	enable_irqs();
	return res;
}

size_t ps2_read(void *dest, size_t n)
{
	char *c = dest;

	size_t nread = 0;
	while (nread < n) {
		int ch = ps2_get();

		if (ch == -1)
			break;
		enum keycode kc = ps2_tokeycode((u8)ch);

		int val = kc_toascii(kc);
		if (val == 0)
			continue;

		/* TODO this should not be here, this driver should also be able
		 * to be used from the kernel */
		c = put_user1(c, (u8) val);
		nread++;
	}
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
	return ps2_tokeycode(ps2_recv());
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
	for (int i = 0; i < 100; i++)
		short_wait();

	int b1 = ps2_get();
	int b2 = ps2_get();

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

void init_ps2(void)
{
	ps2_enable_irq();
	init_ps2_controller();
	ps2_identify();
}

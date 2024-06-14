#include <kernel/kernel.h>
#include <kernel/ps2.h>

static u8 ps2_dev_type1, ps2_dev_type2;

#define SCANCODE_ESC_PRESSED 0x01 // escape pressed
#define SCANCODE_1_PRESSED 0x02 // 1 pressed
#define SCANCODE_2_PRESSED 0x03 // 2 pressed
#define SCANCODE_3_PRESSED 0x04 // 3 pressed
#define SCANCODE_4_PRESSED 0x05 // 4 pressed
#define SCANCODE_5_PRESSED 0x06 // 5 pressed
#define SCANCODE_6_PRESSED 0x07 // 6 pressed
#define SCANCODE_7_PRESSED 0x08 // 7 pressed
#define SCANCODE_8_PRESSED 0x09 // 8 pressed
#define SCANCODE_9_PRESSED 0x0A // 9 pressed
#define SCANCODE_0_PRESSED 0x0B // 0 (zero) pressed
#define SCANCODE_MINUS_PRESSED 0x0C //- pressed
#define SCANCODE_EQUALS_PRESSED 0x0D //= pressed
#define SCANCODE_BACKSPACE_PRESSED 0x0E // backspace pressed
#define SCANCODE_TAB_PRESSED 0x0F // tab pressed
#define SCANCODE_Q_PRESSED 0x10 // Q pressed
#define SCANCODE_W_PRESSED 0x11 // W pressed
#define SCANCODE_E_PRESSED 0x12 // E pressed
#define SCANCODE_R_PRESSED 0x13 // R pressed
#define SCANCODE_T_PRESSED 0x14 // T pressed
#define SCANCODE_Y_PRESSED 0x15 // Y pressed
#define SCANCODE_U_PRESSED 0x16 // U pressed
#define SCANCODE_I_PRESSED 0x17 // I pressed
#define SCANCODE_O_PRESSED 0x18 // O pressed
#define SCANCODE_P_PRESSED 0x19 // P pressed
#define SCANCODE_LBRACK_PRESSED 0x1A //[ pressed
#define SCANCODE_RBRACK_PRESSED 0x1B //] pressed
#define SCANCODE_ENTER_PRESSED 0x1C // enter pressed
#define SCANCODE_LCTRL_PRESSED 0x1D // left control pressed
#define SCANCODE_A_PRESSED 0x1E // A pressed
#define SCANCODE_S_PRESSED 0x1F // S pressed
#define SCANCODE_D_PRESSED 0x20 // D pressed
#define SCANCODE_F_PRESSED 0x21 // F pressed
#define SCANCODE_G_PRESSED 0x22 // G pressed
#define SCANCODE_H_PRESSED 0x23 // H pressed
#define SCANCODE_J_PRESSED 0x24 // J pressed
#define SCANCODE_K_PRESSED 0x25 // K pressed
#define SCANCODE_L_PRESSED 0x26 // L pressed
#define SCANCODE_SEMI_PRESSED 0x27 //; pressed
#define SCANCODE_SINGLE_QUOTE_PRESSED 0x28 //' (single quote) pressed
#define SCANCODE_BACK_TICK_PRESSED 0x29 //` (back tick) pressed
#define SCANCODE_LSHIFT_PRESSED 0x2A // left shift pressed
#define SCANCODE_BACKSLASH_PRESSED 0x2B //\ pressed
#define SCANCODE_Z_PRESSED 0x2C // Z pressed
#define SCANCODE_X_PRESSED 0x2D // X pressed
#define SCANCODE_C_PRESSED 0x2E // C pressed
#define SCANCODE_V_PRESSED 0x2F // V pressed
#define SCANCODE_B_PRESSED 0x30 // B pressed
#define SCANCODE_N_PRESSED 0x31 // N pressed
#define SCANCODE_M_PRESSED 0x32 // M pressed
#define SCANCODE_COMMA_PRESSED 0x33 //, pressed
#define SCANCODE_PERIOD_PRESSED 0x34 //. pressed
#define SCANCODE_FORWARDSLASH_PRESSED 0x35 /// pressed
#define SCANCODE_RSHIFT_PRESSED 0x36 // right shift pressed
#define SCANCODE_KP_STAR_PRESSED 0x37 //(keypad) * pressed
#define SCANCODE_LALT_PRESSED 0x38 // left alt pressed
#define SCANCODE_SPACE_PRESSED 0x39 // space pressed
#define SCANCODE_CAPSLOCK_PRESSED 0x3A // CapsLock pressed
#define SCANCODE_F1_PRESSED 0x3B // F1 pressed
#define SCANCODE_F2_PRESSED 0x3C // F2 pressed
#define SCANCODE_F3_PRESSED 0x3D // F3 pressed
#define SCANCODE_F4_PRESSED 0x3E // F4 pressed
#define SCANCODE_F5_PRESSED 0x3F // F5 pressed
#define SCANCODE_F6_PRESSED 0x40 // F6 pressed
#define SCANCODE_F7_PRESSED 0x41 // F7 pressed
#define SCANCODE_F8_PRESSED 0x42 // F8 pressed
#define SCANCODE_F9_PRESSED 0x43 // F9 pressed
#define SCANCODE_F10_PRESSED 0x44 // F10 pressed
#define SCANCODE_NUMLOCK_PRESSED 0x45 // NumberLock pressed
#define SCANCODE_SCROLLLOCK_PRESSED 0x46 // ScrollLock pressed
#define SCANCODE_KP_7_PRESSED 0x47 //(keypad) 7 pressed
#define SCANCODE_KP_8_PRESSED 0x48 //(keypad) 8 pressed
#define SCANCODE_KP_9_PRESSED 0x49 //(keypad) 9 pressed
#define SCANCODE_KP_MINUS_PRESSED 0x4A //(keypad) - pressed
#define SCANCODE_KP_4_PRESSED 0x4B //(keypad) 4 pressed
#define SCANCODE_KP_5_PRESSED 0x4C //(keypad) 5 pressed
#define SCANCODE_KP_6_PRESSED 0x4D //(keypad) 6 pressed
#define SCANCODE_KP_PLUS_PRESSED 0x4E //(keypad) + pressed
#define SCANCODE_KP_1_PRESSED 0x4F //(keypad) 1 pressed
#define SCANCODE_KP_2_PRESSED 0x50 //(keypad) 2 pressed
#define SCANCODE_KP_3_PRESSED 0x51 //(keypad) 3 pressed
#define SCANCODE_KP_0_PRESSED 0x52 //(keypad) 0 pressed
#define SCANCODE_KP_PERIOD_PRESSED 0x53 //(keypad) . pressed
#define SCANCODE_F11_PRESSED 0x57 // F11 pressed
#define SCANCODE_F12_PRESSED 0x58 // F12 pressed
#define SCANCODE_RELEASED_OFFSET 0x81

#define PS2_TIMEOUT_TRIES 30

static int ps2_recv_timeout(void)
{
	for (int i = 0; i < PS2_TIMEOUT_TRIES; ++i) {
		if (ps2_canrecv())
			return ps2_recv();
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
	}
	return -1;
}

static int ps2_send_ack(u8 b)
{
	for (int i = 0; i < 3; ++i) {
		if (ps2_send_timeout(b) < 0)
			return -1;

		u8 res = ps2_recv_timeout();
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
	switch (ps2_recv()) {
	case SCANCODE_A_PRESSED:
		return KEYCODE_A;
	case SCANCODE_B_PRESSED:
		return KEYCODE_B;
	case SCANCODE_C_PRESSED:
		return KEYCODE_C;
	case SCANCODE_D_PRESSED:
		return KEYCODE_D;
	case SCANCODE_E_PRESSED:
		return KEYCODE_E;
	case SCANCODE_F_PRESSED:
		return KEYCODE_F;
	case SCANCODE_G_PRESSED:
		return KEYCODE_G;
	case SCANCODE_H_PRESSED:
		return KEYCODE_H;
	case SCANCODE_I_PRESSED:
		return KEYCODE_I;
	case SCANCODE_J_PRESSED:
		return KEYCODE_J;
	case SCANCODE_K_PRESSED:
		return KEYCODE_K;
	case SCANCODE_L_PRESSED:
		return KEYCODE_L;
	case SCANCODE_M_PRESSED:
		return KEYCODE_M;
	case SCANCODE_N_PRESSED:
		return KEYCODE_N;
	case SCANCODE_O_PRESSED:
		return KEYCODE_O;
	case SCANCODE_P_PRESSED:
		return KEYCODE_P;
	case SCANCODE_Q_PRESSED:
		return KEYCODE_Q;
	case SCANCODE_R_PRESSED:
		return KEYCODE_R;
	case SCANCODE_S_PRESSED:
		return KEYCODE_S;
	case SCANCODE_T_PRESSED:
		return KEYCODE_T;
	case SCANCODE_U_PRESSED:
		return KEYCODE_U;
	case SCANCODE_V_PRESSED:
		return KEYCODE_V;
	case SCANCODE_W_PRESSED:
		return KEYCODE_W;
	case SCANCODE_X_PRESSED:
		return KEYCODE_X;
	case SCANCODE_Z_PRESSED:
		return KEYCODE_Z;
	case SCANCODE_Y_PRESSED:
		return KEYCODE_Y;
	case SCANCODE_1_PRESSED:
		return KEYCODE_1;
	case SCANCODE_2_PRESSED:
		return KEYCODE_2;
	case SCANCODE_3_PRESSED:
		return KEYCODE_3;
	case SCANCODE_4_PRESSED:
		return KEYCODE_4;
	case SCANCODE_5_PRESSED:
		return KEYCODE_5;
	case SCANCODE_6_PRESSED:
		return KEYCODE_6;
	case SCANCODE_7_PRESSED:
		return KEYCODE_7;
	case SCANCODE_8_PRESSED:
		return KEYCODE_8;
	case SCANCODE_9_PRESSED:
		return KEYCODE_9;
	case SCANCODE_0_PRESSED:
		return KEYCODE_0;
	case SCANCODE_ENTER_PRESSED:
		return KEYCODE_ENTER;
	case SCANCODE_SPACE_PRESSED:
		return KEYCODE_SPACE;
	case SCANCODE_BACKSPACE_PRESSED:
		return KEYCODE_BACKSPACE;
	case SCANCODE_ESC_PRESSED:
	case SCANCODE_MINUS_PRESSED:
	case SCANCODE_EQUALS_PRESSED:
	case SCANCODE_TAB_PRESSED:
	case SCANCODE_LBRACK_PRESSED:
	case SCANCODE_RBRACK_PRESSED:
	case SCANCODE_LCTRL_PRESSED:
		return KEYCODE_LCTRL;
	case SCANCODE_SEMI_PRESSED:
	case SCANCODE_SINGLE_QUOTE_PRESSED:
	case SCANCODE_BACK_TICK_PRESSED:
	case SCANCODE_LSHIFT_PRESSED:
	case SCANCODE_BACKSLASH_PRESSED:
	case SCANCODE_COMMA_PRESSED:
	case SCANCODE_PERIOD_PRESSED:
	case SCANCODE_FORWARDSLASH_PRESSED:
	case SCANCODE_RSHIFT_PRESSED:
	case SCANCODE_KP_STAR_PRESSED:
	case SCANCODE_LALT_PRESSED:
	case SCANCODE_CAPSLOCK_PRESSED:
	case SCANCODE_F1_PRESSED:
		return KEYCODE_F1;
	case SCANCODE_F2_PRESSED:
		return KEYCODE_F2;
	case SCANCODE_F3_PRESSED:
		return KEYCODE_F3;
	case SCANCODE_F4_PRESSED:
		return KEYCODE_F4;
	case SCANCODE_F5_PRESSED:
		return KEYCODE_F5;
	case SCANCODE_F6_PRESSED:
		return KEYCODE_F6;
	case SCANCODE_F7_PRESSED:
		return KEYCODE_F7;
	case SCANCODE_F8_PRESSED:
		return KEYCODE_F8;
	case SCANCODE_F9_PRESSED:
		return KEYCODE_F9;
	case SCANCODE_F10_PRESSED:
		return KEYCODE_F10;
	case SCANCODE_NUMLOCK_PRESSED:
	case SCANCODE_SCROLLLOCK_PRESSED:
	case SCANCODE_KP_7_PRESSED:
	case SCANCODE_KP_8_PRESSED:
	case SCANCODE_KP_9_PRESSED:
	case SCANCODE_KP_MINUS_PRESSED:
	case SCANCODE_KP_4_PRESSED:
	case SCANCODE_KP_5_PRESSED:
	case SCANCODE_KP_6_PRESSED:
	case SCANCODE_KP_PLUS_PRESSED:
	case SCANCODE_KP_1_PRESSED:
	case SCANCODE_KP_2_PRESSED:
	case SCANCODE_KP_3_PRESSED:
	case SCANCODE_KP_0_PRESSED:
	case SCANCODE_KP_PERIOD_PRESSED:
	case SCANCODE_F11_PRESSED:
		return KEYCODE_F11;
	case SCANCODE_F12_PRESSED:
		return KEYCODE_F12;
	default:
		return KEYCODE_UNKNOWN;
	}
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

static void ps2_init_controller(void)
{
	//TODO check if controller exists once we are using ACPI
	//TODO also properly test port2
	
	if (ps2_send_cmd_timeout(PS2_CMD_DISABLE_PORT1)) {
		printk("failed to disable ps2 port1\n");
		return;
	}

	if (ps2_send_cmd_timeout(PS2_CMD_DISABLE_PORT2)) {
		printk("failed to disable ps2 port2\n");
		return;
	}

	//flush output buffer, we don't care what we got
	ps2_recv();

	if (ps2_send_cmd_timeout(PS2_CMD_READ_CONF_BYTE)) {
		printk("failed to get ps2 config byte\n");
		return;
	}
	u8 conf = ps2_recv_timeout();

	//disable interrupts
	conf &= ~(PS2_CONF_PORT1_IRQ | PS2_CONF_PORT2_IRQ);

	if (ps2_send_cmd_timeout(PS2_CMD_WRITE_CONF_BYTE)) {
		printk("failed to write ps2 config byte (1)\n");
		return;
	}

	if (ps2_send_timeout(conf) < 0) {
		printk("failed to write ps2 config byte (2)\n");
		return;
	}

	//apparently this might reset the controller, if you have issues with
	//the keyboard, it's probably a good idea to check here first
	if (ps2_send_cmd_timeout(PS2_CMD_TEST_CONTROLLER)) {
		printk("failed to test controller\n");
		return;
	}
	u8 res = ps2_recv_timeout();

	if (res != PS2_CONTROLLER_OK) {
		printk("ps2 controller self test failed");
		return;
	}

	if (ps2_send_cmd_timeout(PS2_CMD_TEST_PORT1)) {
		printk("ps2: failed to test port1\n");
		return;
	}
	res = ps2_recv_timeout();

	if (res != PS2_PORT_OK) {
		printk("ps2 port1 self test failed");
		return;
	}

	if (ps2_send_cmd_timeout(PS2_CMD_ENABLE_PORT1)) {
		printk("ps2: failed to enable port1");
		return;
	}

	ps2_send_timeout(PS2_RESET);
	if (ps2_send_ack(PS2_RESET) < 0) {
		printk("failed to reset device on port1");
		return;
	}

	if (ps2_recv_timeout() != PS2_PASSED_SELFTEST) {
		printk("failed selftest after resetting device");
		return;
	}
}

static void ps2_init_devices(void)
{
	if (ps2_send_ack(PS2_DISABLE_SCANNING) < 0) {
		printk("failed to disable scanning\n");
		return;
	}

	if (ps2_send_ack(PS2_IDENTIFY) < 0) {
		printk("failed to identify ps/2 device\n");
		return;
	}

	ps2_dev_type1 = ps2_dev_type2 = 0;

	int res = ps2_recv_timeout();
	if (res > 0) {
		ps2_dev_type1 = (u8)res;

		res = ps2_recv_timeout();
		if (res > 0) {
			ps2_dev_type2 = (u8)res;
		}
	}

	printk("identified keyboard: 0x%x 0x%x\n", ps2_dev_type1,
	       ps2_dev_type2);

	if (ps2_send_ack(PS2_ENABLE_SCANNING) < 0)
		printk("failed to re-enable scanning\n");
}

void ps2_init(void)
{
	ps2_init_controller();
	ps2_init_devices();
}

#ifndef __KERNEL_PS2_H
#define __KERNEL_PS2_H

#include <stdbool.h>

#include <kernel/keycode.h>
#include <kernel/types.h>

#define PS2_IDENTIFY 0xF2
#define PS2_ENABLE_SCANNING 0xF4
#define PS2_DISABLE_SCANNING 0xF5
#define PS2_ACK 0xFA
#define PS2_RESEND 0xFE
#define PS2_RESET 0xFF

#define PS2_PASSED_SELFTEST 0xAA

#define PS2_CMD_DISABLE_PORT1 0xAD
#define PS2_CMD_ENABLE_PORT1 0xAE
#define PS2_CMD_DISABLE_PORT2 0xA7
#define PS2_CMD_READ_CONF_BYTE 0x20
#define PS2_CMD_WRITE_CONF_BYTE 0x60
#define PS2_CMD_TEST_CONTROLLER 0xAA
#define PS2_CMD_TEST_PORT1 0xAB
#define PS2_CMD_RESET 0xFE

#define PS2_CONTROLLER_OK 0x55
#define PS2_PORT_OK 0x00

#define PS2_CONF_PORT1_IRQ 0b00000001
#define PS2_CONF_PORT2_IRQ 0b00000010
#define PS2_CONF_SYS_FLAG 0b00000100
#define PS2_CONF_PORT1_CLOCK 0b00010000
#define PS2_CONF_PORT2_CLOCK 0b00100000
#define PS2_CONF_PORT1_TRANSLATE 0b01000000

#define PS2_SCANCODE_ESC_PRESSED 0x01 // escape pressed
#define PS2_SCANCODE_1_PRESSED 0x02 // 1 pressed
#define PS2_SCANCODE_2_PRESSED 0x03 // 2 pressed
#define PS2_SCANCODE_3_PRESSED 0x04 // 3 pressed
#define PS2_SCANCODE_4_PRESSED 0x05 // 4 pressed
#define PS2_SCANCODE_5_PRESSED 0x06 // 5 pressed
#define PS2_SCANCODE_6_PRESSED 0x07 // 6 pressed
#define PS2_SCANCODE_7_PRESSED 0x08 // 7 pressed
#define PS2_SCANCODE_8_PRESSED 0x09 // 8 pressed
#define PS2_SCANCODE_9_PRESSED 0x0A // 9 pressed
#define PS2_SCANCODE_0_PRESSED 0x0B // 0 (zero) pressed
#define PS2_SCANCODE_MINUS_PRESSED 0x0C //- pressed
#define PS2_SCANCODE_EQUALS_PRESSED 0x0D //= pressed
#define PS2_SCANCODE_BACKSPACE_PRESSED 0x0E // backspace pressed
#define PS2_SCANCODE_TAB_PRESSED 0x0F // tab pressed
#define PS2_SCANCODE_Q_PRESSED 0x10 // Q pressed
#define PS2_SCANCODE_W_PRESSED 0x11 // W pressed
#define PS2_SCANCODE_E_PRESSED 0x12 // E pressed
#define PS2_SCANCODE_R_PRESSED 0x13 // R pressed
#define PS2_SCANCODE_T_PRESSED 0x14 // T pressed
#define PS2_SCANCODE_Y_PRESSED 0x15 // Y pressed
#define PS2_SCANCODE_U_PRESSED 0x16 // U pressed
#define PS2_SCANCODE_I_PRESSED 0x17 // I pressed
#define PS2_SCANCODE_O_PRESSED 0x18 // O pressed
#define PS2_SCANCODE_P_PRESSED 0x19 // P pressed
#define PS2_SCANCODE_LBRACK_PRESSED 0x1A //[ pressed
#define PS2_SCANCODE_RBRACK_PRESSED 0x1B //] pressed
#define PS2_SCANCODE_ENTER_PRESSED 0x1C // enter pressed
#define PS2_SCANCODE_LCTRL_PRESSED 0x1D // left control pressed
#define PS2_SCANCODE_A_PRESSED 0x1E // A pressed
#define PS2_SCANCODE_S_PRESSED 0x1F // S pressed
#define PS2_SCANCODE_D_PRESSED 0x20 // D pressed
#define PS2_SCANCODE_F_PRESSED 0x21 // F pressed
#define PS2_SCANCODE_G_PRESSED 0x22 // G pressed
#define PS2_SCANCODE_H_PRESSED 0x23 // H pressed
#define PS2_SCANCODE_J_PRESSED 0x24 // J pressed
#define PS2_SCANCODE_K_PRESSED 0x25 // K pressed
#define PS2_SCANCODE_L_PRESSED 0x26 // L pressed
#define PS2_SCANCODE_SEMI_PRESSED 0x27 //; pressed
#define PS2_SCANCODE_SINGLE_QUOTE_PRESSED 0x28 //' (single quote) pressed
#define PS2_SCANCODE_BACK_TICK_PRESSED 0x29 //` (back tick) pressed
#define PS2_SCANCODE_LSHIFT_PRESSED 0x2A // left shift pressed
#define PS2_SCANCODE_BACKSLASH_PRESSED 0x2B //\ pressed
#define PS2_SCANCODE_Z_PRESSED 0x2C // Z pressed
#define PS2_SCANCODE_X_PRESSED 0x2D // X pressed
#define PS2_SCANCODE_C_PRESSED 0x2E // C pressed
#define PS2_SCANCODE_V_PRESSED 0x2F // V pressed
#define PS2_SCANCODE_B_PRESSED 0x30 // B pressed
#define PS2_SCANCODE_N_PRESSED 0x31 // N pressed
#define PS2_SCANCODE_M_PRESSED 0x32 // M pressed
#define PS2_SCANCODE_COMMA_PRESSED 0x33 //, pressed
#define PS2_SCANCODE_PERIOD_PRESSED 0x34 //. pressed
#define PS2_SCANCODE_FORWARDSLASH_PRESSED 0x35 /// pressed
#define PS2_SCANCODE_RSHIFT_PRESSED 0x36 // right shift pressed
#define PS2_SCANCODE_KP_STAR_PRESSED 0x37 //(keypad) * pressed
#define PS2_SCANCODE_LALT_PRESSED 0x38 // left alt pressed
#define PS2_SCANCODE_SPACE_PRESSED 0x39 // space pressed
#define PS2_SCANCODE_CAPSLOCK_PRESSED 0x3A // CapsLock pressed
#define PS2_SCANCODE_F1_PRESSED 0x3B // F1 pressed
#define PS2_SCANCODE_F2_PRESSED 0x3C // F2 pressed
#define PS2_SCANCODE_F3_PRESSED 0x3D // F3 pressed
#define PS2_SCANCODE_F4_PRESSED 0x3E // F4 pressed
#define PS2_SCANCODE_F5_PRESSED 0x3F // F5 pressed
#define PS2_SCANCODE_F6_PRESSED 0x40 // F6 pressed
#define PS2_SCANCODE_F7_PRESSED 0x41 // F7 pressed
#define PS2_SCANCODE_F8_PRESSED 0x42 // F8 pressed
#define PS2_SCANCODE_F9_PRESSED 0x43 // F9 pressed
#define PS2_SCANCODE_F10_PRESSED 0x44 // F10 pressed
#define PS2_SCANCODE_NUMLOCK_PRESSED 0x45 // NumberLock pressed
#define PS2_SCANCODE_SCROLLLOCK_PRESSED 0x46 // ScrollLock pressed
#define PS2_SCANCODE_KP_7_PRESSED 0x47 //(keypad) 7 pressed
#define PS2_SCANCODE_KP_8_PRESSED 0x48 //(keypad) 8 pressed
#define PS2_SCANCODE_KP_9_PRESSED 0x49 //(keypad) 9 pressed
#define PS2_SCANCODE_KP_MINUS_PRESSED 0x4A //(keypad) - pressed
#define PS2_SCANCODE_KP_4_PRESSED 0x4B //(keypad) 4 pressed
#define PS2_SCANCODE_KP_5_PRESSED 0x4C //(keypad) 5 pressed
#define PS2_SCANCODE_KP_6_PRESSED 0x4D //(keypad) 6 pressed
#define PS2_SCANCODE_KP_PLUS_PRESSED 0x4E //(keypad) + pressed
#define PS2_SCANCODE_KP_1_PRESSED 0x4F //(keypad) 1 pressed
#define PS2_SCANCODE_KP_2_PRESSED 0x50 //(keypad) 2 pressed
#define PS2_SCANCODE_KP_3_PRESSED 0x51 //(keypad) 3 pressed
#define PS2_SCANCODE_KP_0_PRESSED 0x52 //(keypad) 0 pressed
#define PS2_SCANCODE_KP_PERIOD_PRESSED 0x53 //(keypad) . pressed
#define PS2_SCANCODE_F11_PRESSED 0x57 // F11 pressed
#define PS2_SCANCODE_F12_PRESSED 0x58 // F12 pressed
#define PS2_SCANCODE_RELEASED_OFFSET 0x81

void ps2_send(u8 b);
void ps2_send_cmd(u8 b);
u8 ps2_recv(void);

bool ps2_cansend(void);
bool ps2_canrecv(void);

/* returns the irq number */
i16 ps2_setup_irq(void);
void ps2_eoi(void);


void init_ps2(void);
__attribute__((deprecated("will soon use new interrupt based driver")))
enum keycode ps2_getkey_timeout(void);

#endif

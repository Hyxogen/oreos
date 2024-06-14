#ifndef _KERNEL_PS2_H
#define _KERNEL_PS2_H

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

void ps2_send(u8 b);
void ps2_send_cmd(u8 b);
u8 ps2_recv(void);

bool ps2_cansend(void);
bool ps2_canrecv(void);

void ps2_init(void);

enum keycode ps2_getkey_timeout(void);

#endif

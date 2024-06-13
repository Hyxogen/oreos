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

void ps2_send(u8 b);
u8 ps2_recv(void);

bool ps2_cansend(void);
bool ps2_canrecv(void);

void ps2_init(void);

enum keycode ps2_getkey_timeout(void);

#endif

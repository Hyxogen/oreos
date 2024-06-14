NAME		:= oreos

ISO		:= $(NAME).iso
KERNEL		:= $(NAME).bin

CC		:= $(HOME)/opt/cross/bin/i686-elf-gcc
CFLAGS		:= -std=c11 -O0 -Wall -Wextra -ffreestanding -masm=intel -g -Iinclude -MMD -Iinclude -Ilibc/include

AS		:= nasm
ASFLAGS		:= -felf32

LD_CMD		:= $(CC)
LFLAGS		:= -ffreestanding -O2 -nostdlib -lgcc

ISO_DIR		:= isodir/

OBJS		:=
DEPS		:=

all: $(ISO)

#non-recursive make
#http://sites.e-advies.nl/nonrecursive-make.html
dir		:= arch
include		$(dir)/Rules.mk
dir		:= libc
include		$(dir)/Rules.mk
dir		:= kernel
include		$(dir)/Rules.mk

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

%.o: %.asm
	$(AS) $(ASFLAGS) $< -o $@

$(ISO): $(KERNEL) grub.cfg
	mkdir -p $(ISO_DIR)/boot/grub
	cp grub.cfg $(ISO_DIR)/boot/grub
	cp $(KERNEL) $(ISO_DIR)/boot
	grub-mkrescue -o $@ $(ISO_DIR)

$(KERNEL): $(OBJS) Makefile
	$(LD_CMD) -o $@ $(OBJS) $(LFLAGS)

run: $(ISO)
	qemu-system-i386 -cdrom $(ISO)

debug: $(ISO)
	qemu-system-i386 -cdrom $(ISO) -s -S &
	gdb

clean:
	rm -f $(CLEAN)
	rm -rf $(ISO_DIR)
	@${MAKE} -C kernel clean

.PHONY: all run debug clean $(ISO)

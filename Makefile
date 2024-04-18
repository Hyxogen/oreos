NAME		:= oreos
AS		:= nasm

ISO		:= $(NAME).iso
KERNEL		:= $(NAME).bin

CC		:= $(HOME)/opt/cross/bin/i686-elf-gcc
CFLAGS		:= -std=c11 -O0 -Wall -Wextra -ffreestanding -masm=intel -g

OBJ_DIR		:= build

C_FILES		:= kernel.c
ASM_FILES	:= boot.asm
OBJ_FILES	:= $(patsubst %.c,$(OBJ_DIR)/%.o,$(C_FILES)) $(patsubst %.asm,$(OBJ_DIR)/%.o,$(ASM_FILES))
DEP_FILES	:= $(patsubst %.c,$(OBJ_DIR)/%.d,$(C_FILES))

all: $(ISO)

$(ISO): $(KERNEL) grub.cfg Makefile
	rm -rf isodir
	mkdir -p isodir/boot/grub
	cp grub.cfg isodir/boot/grub
	cp $(KERNEL) isodir/boot
	grub-mkrescue -o $@ isodir

$(KERNEL): $(OBJ_FILES) linker.ld Makefile
	$(CC) -T linker.ld -o $(KERNEL) $(OBJ_FILES) -ffreestanding -O2 -nostdlib -lgcc

$(OBJ_DIR)/%.o: %.asm Makefile
	@mkdir -p $(@D)
	$(AS) -felf32 $< -o $@

$(OBJ_DIR)/%.o: %.c Makefile
	@mkdir -p $(@D)
	$(CC) -c $< -o $@ $(CFLAGS)

run: $(ISO)
	qemu-system-i386 -cdrom $(ISO) -nographic

clean:
	rm -rf isodir
	rm -rf build
	rm -f $(KERNEL) $(ISO)

.PHONY: all run

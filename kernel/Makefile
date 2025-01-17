NAME		?= oreos
AS		:= nasm

KERNEL		:= $(NAME).bin

CC		:= $(HOME)/opt/cross/bin/i686-elf-gcc
CFLAGS		:= -std=c11 -O0 -Wall -Wextra -ffreestanding -masm=intel -g -Iinclude -MMD

OBJ_DIR		:= build

I386_DIR	:= i386

C_FILES		:= $(I386_DIR)/gdt.c $(I386_DIR)/framebuf.c $(I386_DIR)/platform.c $(I386_DIR)/ps2.c lib.c printk.c kernel.c ps2.c keycode.c tty.c psf.c
ASM_FILES	:= $(I386_DIR)/boot.asm
OBJ_FILES	:= $(patsubst %.c,$(OBJ_DIR)/%.o,$(C_FILES)) $(patsubst %.asm,$(OBJ_DIR)/%.o,$(ASM_FILES))
DEP_FILES	:= $(patsubst %.c,$(OBJ_DIR)/%.d,$(C_FILES))

all: $(KERNEL)

$(KERNEL): $(OBJ_FILES) $(I386_DIR)/linker.ld Makefile
	$(CC) -T $(I386_DIR)/linker.ld -o $@ $(OBJ_FILES) $(I386_DIR)/font.o -ffreestanding -O2 -nostdlib -lgcc

$(OBJ_DIR)/%.o: %.asm Makefile
	@mkdir -p $(@D)
	$(AS) -felf32 $< -o $@

$(OBJ_DIR)/%.o: %.c Makefile
	@mkdir -p $(@D)
	$(CC) -c $< -o $@ $(CFLAGS)

clean:
	rm -rf build
	rm -f $(KERNEL)

fmt:
	clang-format -i $(C_FILES) $(shell find include/ -type f -name '*.h')

-include $(DEP_FILES)

.PHONY: all clean fmt

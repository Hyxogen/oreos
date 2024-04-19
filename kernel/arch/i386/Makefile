NAME		:= arch.a

CC		?= $(HOME)/opt/cross/bin/i686-elf-gcc
CFLAGS		:= -std=c11 -O0 -Wall -Wextra -ffreestanding -masm=intel -g -I../../include
AS		:= nasm

OBJ_DIR		:= build

C_FILES		:= kernel.c
ASM_FILES	:= boot.asm
OBJ_FILES	:= $(patsubst %.c,$(OBJ_DIR)/%.o,$(C_FILES)) $(patsubst %.asm,$(OBJ_DIR)/%.o,$(ASM_FILES))
DEP_FILES	:= $(patsubst %.c,$(OBJ_DIR)/%.d,$(C_FILES))

OBJ_DIR		:= build

C_FILES		:= tty.c platform.c
ASM_FILES	:= boot.asm
OBJ_FILES	:= $(patsubst %.c,$(OBJ_DIR)/%.o,$(C_FILES)) $(patsubst %.asm,$(OBJ_DIR)/%.o,$(ASM_FILES))
DEP_FILES	:= $(patsubst %.c,$(OBJ_DIR)/%.d,$(C_FILES))

all: $(NAME)

$(NAME): $(OBJ_FILES)
	ar rcs $@ $(OBJ_FILES)

$(OBJ_DIR)/%.o: %.asm Makefile
	@mkdir -p $(@D)
	$(AS) -felf32 $< -o $@

$(OBJ_DIR)/%.o: %.c Makefile
	@mkdir -p $(@D)
	$(CC) -c $< -o $@ $(CFLAGS)

clean:
	rm -rf build
	rm -f $(NAME)

.PHONY: all clean

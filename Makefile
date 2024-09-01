NAME		:= oreos

ISO		:= $(NAME).iso
KERNEL		:= $(NAME).bin

KBD_VER		:= 2.6.4
KBD_SHA256	:= 519f8d087aecca7e0a33cd084bef92c066eb19731666653dcc70c9d71aa40926
CONSOLEFONT	:= lat0-08.psfu

CC		:= $(HOME)/opt/cross/bin/i686-elf-gcc
CFLAGS		:= \
		   -std=c11 -Og -Wall -Wextra -ffreestanding \
		   -fno-builtin -fno-exceptions -fno-stack-protector \
		   -nostdlib -nodefaultlibs \
		   -masm=intel -g3 -Iinclude -MMD -Iinclude \
		   -Ilib/include

AS		:= nasm
ASFLAGS		:= -felf32 -g

LD_CMD		:= $(CC)
LFLAGS		:= -ffreestanding -O2 -nostdlib -lgcc

ISO_DIR		:= isodir/

OBJ_DIR		:= build
OBJS		:=
DEPS		:=

VENDOR_DIR	:= vendor

all: $(ISO)

#non-recursive make
#http://sites.e-advies.nl/nonrecursive-make.html
dir		:= lib
include		$(dir)/Rules.mk
dir		:= kernel
include		$(dir)/Rules.mk
dir		:= boot
include		$(dir)/Rules.mk

OBJS		:= $(addprefix $(OBJ_DIR)/, $(OBJS))
DEPS		:= $(addprefix $(OBJ_DIR)/, $(DEPS))

$(OBJ_DIR)/%.o: %.c Makefile
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJ_DIR)/%.o: %.asm Makefile $(@D)
	@mkdir -p $(@D)
	$(AS) $(ASFLAGS) $< -o $@

$(ISO): $(KERNEL) grub.cfg
	@echo $(DEPS)
	mkdir -p $(ISO_DIR)/boot/grub
	cp grub.cfg $(ISO_DIR)/boot/grub
	cp $(KERNEL) $(ISO_DIR)/boot
	grub-mkrescue -o $@ $(ISO_DIR)

$(KERNEL): $(CONSOLEFONT) $(OBJS) Makefile
	$(LD_CMD) -o $@ $(OBJS) $(LFLAGS)

$(VENDOR_DIR)/kbd-$(KBD_VER):
	mkdir -p $(@D)
	wget --no-clobber -P $(VENDOR_DIR) "https://mirrors.edge.kernel.org/pub/linux/utils/kbd/kbd-$(KBD_VER).tar.xz"
	echo "$(KBD_SHA256) $(VENDOR_DIR)/kbd-$(KBD_VER).tar.xz" > $(VENDOR_DIR)/kbd-sha256sum.txt
	sha256sum --strict --check $(VENDOR_DIR)/kbd-sha256sum.txt
	rm -f kbd-sha256sum.txt
	tar -C $(VENDOR_DIR) -xvf $(VENDOR_DIR)/kbd-$(KBD_VER).tar.xz > /dev/null
	rm -f $(VENDOR_DIR)/kbd-$(KBD_VER).tar.xz
	rm -f $(VENDOR_DIR)/kbd-sha256sum.txt

$(CONSOLEFONT): $(VENDOR_DIR)/kbd-$(KBD_VER)
	cp $(VENDOR_DIR)/kbd-$(KBD_VER)/data/consolefonts/$(CONSOLEFONT) .

run: $(ISO)
	qemu-system-i386 -m 2G -machine pc-q35-9.0 -cdrom $(ISO)

debug: $(ISO)
	qemu-system-i386 -cdrom $(ISO) -s -S &
	gdb

clean:
	rm -rf $(OBJ_DIR)
	rm -rf $(ISO_DIR)
	rm -rf $(KERNEL)

fclean:
	${MAKE} clean
	rm -f  $(ISO)

-include $(DEPS)
.PHONY: all run debug clean $(ISO)

NAME	:= oreos

ISO	:= $(NAME).iso
KERNEL	:= $(NAME).bin

ISO_DIR	:= isodir/

all: $(ISO)

$(ISO): grub.cfg
	@${MAKE} -C kernel
	mkdir -p $(ISO_DIR)/boot/grub
	cp grub.cfg $(ISO_DIR)/boot/grub
	cp kernel/$(KERNEL) $(ISO_DIR)/boot
	grub-mkrescue -o $@ $(ISO_DIR)

run: $(ISO)
	qemu-system-i386 -cdrom $(ISO)

debug: $(ISO)
	qemu-system-i386 -cdrom $(ISO) -s -S

clean:
	rm -rf $(ISO_DIR)
	@${MAKE} -C kernel clean

.PHONY: all run debug clean

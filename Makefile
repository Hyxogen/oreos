AS	:= nasm

CC	:= $(HOME)/opt/cross/bin/i686-elf-gcc
CFLAGS	:= -std=c11 -O2 -Wall -Wextra -ffreestanding

all: hyxos.iso

hyxos.iso: hyxos.bin grub.cfg Makefile
	rm -rf isodir
	mkdir -p isodir/boot/grub
	cp grub.cfg isodir/boot/grub
	cp hyxos.bin isodir/boot
	grub-mkrescue -o $@ isodir

hyxos.bin: boot.o kernel.o linker.ld Makefile
	$(CC) -T linker.ld -o hyxos.bin kernel.o -ffreestanding -O2 -nostdlib boot.o -lgcc

%.o: %.asm Makefile
	$(AS) -felf32 $< -o $@

%.o: %.c Makefile
	$(CC) -c $< -o $@ $(CFLAGS)

run: hyxos.iso
	qemu-system-i386 -cdrom hyxos.iso -nographic

clean:
	rm -rf isodir
	rm -f boot.o hyxos.bin hyxos.iso

.PHONY: all run

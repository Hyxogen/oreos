MULTIBOOT2_MAGIC	equ 0xE85250D6
I386_ARCH		equ 0

section .multiboot
multiboot_hdr_beg:
align 8
	dd MULTIBOOT2_MAGIC
	dd I386_ARCH

	;multiboot2 header size
	dd multiboot_hdr_end - multiboot_hdr_beg
	;multiboot2 checksum
	dd -(MULTIBOOT2_MAGIC + I386_ARCH + (multiboot_hdr_end - multiboot_hdr_beg))

	; empty tag to indicate last tag
	dw 0
	dw 0
	dd 8
multiboot_hdr_end:

section .bss
align 16
stack_bot:
resb 0x4000 ; allocate 16 KiB for stack
stack_top:

multiboot_boot_info:
resb 4 ; address to store multiboot boot information

section .text
global _start:function (_start.end - _start)
_start:
	mov esp, stack_top

	mov [multiboot_boot_info], ebx

	extern kernel_main
	call kernel_main

	cli
.hang:	hlt
	jmp .hang
.end:

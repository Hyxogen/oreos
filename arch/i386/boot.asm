MULTIBOOT2_MAGIC	equ 0xE85250D6
I386_ARCH		equ 0

MULTIBOOT2_HEADER_TAG_END equ 0
MULTIBOOT2_HEADER_TAG_FRAMEBUFFER equ 5
MULTIBOOT2_HEADER_TAG_OPTIONAL equ 1

section .multiboot
multiboot_hdr_beg:
align 8
	dd MULTIBOOT2_MAGIC
	dd I386_ARCH
	;multiboot2 header size
	dd multiboot_hdr_end - multiboot_hdr_beg
	;multiboot2 checksum
	dd -(MULTIBOOT2_MAGIC + I386_ARCH + (multiboot_hdr_end - multiboot_hdr_beg))

framebuffer_tag_beg:
align 8
	dw MULTIBOOT2_HEADER_TAG_FRAMEBUFFER
	dw MULTIBOOT2_HEADER_TAG_OPTIONAL
	dd framebuffer_tag_end - framebuffer_tag_beg
	dd 1012
	dd 768
	dd 32
framebuffer_tag_end:
align 8
	dw MULTIBOOT2_HEADER_TAG_END
	dw 0
	dd 8
multiboot_hdr_end:

section .gdt
gdtd:
	dw 0
	dd 0

section .rodata
global _binary_font_psfu_start, _binary_font_psfu_end
_binary_font_psfu_start:
incbin "lat0-08.psfu"
_binary_font_psfu_end:

section .bss
align 16
stack_bot:
resb 0x4000 ; allocate 16 KiB for stack
stack_top:

section .text

global _idle:function (_idle.end - _idle)
_idle:
	cli
.hang:	hlt
	jmp .hang
.end:

global _load_gdt:function (_load_gdt.end - _load_gdt)
_load_gdt:
	mov eax, [esp + 4]
	mov [gdtd + 2], ax
	mov ax, [esp + 8]
	mov [gdtd], ax
	lgdt [gdtd]
	ret
.end:

global _start:function (_start.end - _start)
_start:
	mov esp, stack_top


	; clear eflags
	push dword 0
	popf

	push ebx ; pass multiboot info struct as first argument
	extern _term_init
	call _term_init
	pop ebx

	extern kernel_main
	call kernel_main

	call _idle
.end:

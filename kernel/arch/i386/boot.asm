%define MB2_MAGIC			0xe85250d6
%define MB2_ARCH_I386			0
%define MB2_HEADER_TAG_END		0
%define MB2_HEADER_TAG_FRAMEBUFFER	5
%define MB2_HEADER_TAG_OPTIONAL		1

; MULTIBOOT HEADER
section .multiboot.rodata
mb2_hdr_beg:
align 8
	dd MB2_MAGIC
	dd MB2_ARCH_I386
	;multiboot2 header size
	dd mb2_hdr_end - mb2_hdr_beg
	;multiboot2 checksum
	dd -(MB2_MAGIC + MB2_ARCH_I386 + (mb2_hdr_end - mb2_hdr_beg))

mb2_framebuf_tag_beg:
align 8
	dw MB2_HEADER_TAG_FRAMEBUFFER
	dw MB2_HEADER_TAG_OPTIONAL
	dd mb2_framebuf_tag_end - mb2_framebuf_tag_beg
	dd 1012
	dd 768
	dd 32
mb2_framebuf_tag_end:
align 8
	dw MB2_HEADER_TAG_END
	dw 0
	dd 8
mb2_hdr_end:

section .bss
align 16
_stack_bot:
	resb 0x4000 ; allocate 16 KiB for stack
global _stack_top
_stack_top:

extern _kernel_start, _kernel_end, _kernel_addr

section .multiboot.text
global _start:function (_start.end - _start)
_start:
	; multiboot has left our eflags undefined, clear them
	push dword 0
	popf

	; setup temporary stack
	mov esp, _stack_top
	sub esp, _kernel_addr

	push ebx ; store multiboot info struct pointer

	extern setup_paging
	call setup_paging
.end:

global enable_paging_and_jump_to_kmain:function (enable_paging_and_jump_to_kmain.end - enable_paging_and_jump_to_kmain)
enable_paging_and_jump_to_kmain:
	mov eax, cr0
	or eax, 0x80010001
	mov cr0, eax

	jmp _start_paged
.end:

section .text
_start_paged:
	; setup stack with virtual address
	mov esp, _stack_top

	extern _multiboot_info
	push dword [_multiboot_info]

	; zero ebp for stack unwinding
	xor ebp, ebp

	extern kernel_main
	call kernel_main

	; we're done
	call idle

.end:

global idle:function (idle.end - idle)
idle:
	cli
.hang:	hlt
	jmp .hang
.end:

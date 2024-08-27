MULTIBOOT2_MAGIC	equ 0xE85250D6
I386_ARCH		equ 0

MULTIBOOT2_HEADER_TAG_END equ 0
MULTIBOOT2_HEADER_TAG_FRAMEBUFFER equ 5
MULTIBOOT2_HEADER_TAG_OPTIONAL equ 1
KERNEL_PAGESIZE	equ 4096
KERNEL_PTE_SIZE	equ 4

; MULTIBOOT HEADER
section .multiboot.rodata
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

section .rodata
global _binary_font_psfu_start, _binary_font_psfu_end
_binary_font_psfu_start:
incbin "lat0-08.psfu"
_binary_font_psfu_end:

section .bss
align 16
gdtd:
	resb 6

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
	; Unmap identity mapping
	; mov [_page_dir], dword 0
	; call _flush_tlb

	extern kernel_main
	call kernel_main

	; we're done
	call _idle
.end:

global _idle:function (_idle.end - _idle)
_idle:
	cli
.hang:	hlt
	jmp .hang
.end:

global _load_gdt:function (_load_gdt.end - _load_gdt)
_load_gdt:
	;TODO make sure that interrupts are disabled
	mov eax, [esp + 4]
	mov [gdtd + 2], eax
	mov ax, [esp + 8]
	mov [gdtd], ax
	lgdt [gdtd]
.reload_segments:
	jmp 0x08:.reload_cs
.reload_cs:
	mov ax, 0x10
	mov ds, ax
	mov es, ax
	mov fs, ax
	mov gs, ax
	mov ss, ax
	ret
.end:

global _flush_tlb:function (_flush_tlb.end - _flush_tlb)
_flush_tlb:
	mov eax, cr3
	mov cr3, eax
	ret
.end:

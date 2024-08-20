MULTIBOOT2_MAGIC	equ 0xE85250D6
I386_ARCH		equ 0

MULTIBOOT2_HEADER_TAG_END equ 0
MULTIBOOT2_HEADER_TAG_FRAMEBUFFER equ 5
MULTIBOOT2_HEADER_TAG_OPTIONAL equ 1
KERNEL_ADDR equ 0xC0000000
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

_page_dir:
	align 0x1000
	resb 0x1000
_page_table1:
	align 0x1000
	resb 0x1000

align 16

_stack_bot:
	resb 0x4000 ; allocate 16 KiB for stack
global _stack_top
_stack_top:

extern _kernel_start, _kernel_end

;TODO the start function needs to be placed differently than all the other
;functions
section .multiboot.text
global _start:function (_start.end - _start)
_start:
	; setup temporary stack
	mov esp, _stack_top - KERNEL_ADDR

	; multiboot has left our eflags undefined, clear them
	push dword 0
	popf

	push dword [ebx] ; store multiboot info struct size
	push ebx ; store multiboot info struct pointer

	; sanity check, make sure the kernel is not more than 1 MiB as the
	; bootstrap code does not currently support it
	mov eax, _kernel_end
	sub eax, _kernel_start
	cmp eax, 0x100000
	jle .setup

	jmp _idle - KERNEL_ADDR

.setup:
	; setup loop variables
	xor eax, eax ; current address
	mov ebx, _page_table1 - KERNEL_ADDR ; current PTE address

.loop:
	cmp eax, _kernel_start
	jl .next_page

	cmp eax, _kernel_end - KERNEL_ADDR
	jge .done

	mov edx, eax
	or edx, 0b0011 ; enable read/write and present
	; TODO do not map rodata and text as write
	mov [ebx], edx

.next_page:
	add eax, KERNEL_PAGESIZE
	add ebx, KERNEL_PTE_SIZE
	jmp .loop

.done:
	; done setting up PTEs

	; map the PDEs
	mov eax, _page_table1 - KERNEL_ADDR + 0b0011
	mov [_page_dir - KERNEL_ADDR + 0], eax
	mov [_page_dir - KERNEL_ADDR + 768 * KERNEL_PTE_SIZE], eax
	; setup recursive page table
	mov dword [_page_dir - KERNEL_ADDR + 1023 * KERNEL_PTE_SIZE], _page_dir - KERNEL_ADDR + 0b0011

	; set cr3 to the address of the page table directory
	lea eax, [_page_dir - KERNEL_ADDR]
	mov cr3, eax

	; enable paging
	mov eax, cr0
	or eax, 0x80000001
	mov cr0, eax

	; jump to virtual address
	jmp _start_paged
.end:

section .text
_start_paged:
	; setup stack with virtual address
	add esp, KERNEL_ADDR

	; Unmap identity mapping
	mov [_page_dir], dword 0
	call _flush_tlb

	; we're using paging now, start early initialization
	extern early_main
	call early_main

	; early init done, call main
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
	mov [gdtd + 2], ax
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

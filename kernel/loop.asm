; TODO remove
global loop:function (loop.end - loop)
align 0x1000
loop:
;	mov eax, 0x5a ; mmap syscall
;	mov ebx, 0x0 ; NULL
;	mov ecx, 0x1000 ; 1KB
;	mov edx, 0x3 ; PROT_READ | PROT_WRITE
;	mov esi, 0x3 ; MAP_PRIVATE | MAP_ANONYMOUS
;	mov edi, 0x0 ; fd = 0
;	mov ebp, 0x0 ; off = 0
;
;	int 0x80
;
;	test eax, -1
;	jne .start
;	xchg bx, bx
;
;.start:
;	mov esp, eax
;	add esp, 0x1000 ; move esp to stack top

	sub esp, 0x1 ; allocate 1 byte on the stack
.loop:
	mov eax, 0x03 ; read syscall
	mov ebx, 0x00 ; fd 0
	mov ecx, esp ; buf
	mov edx, 1 ; len

	int 0x80

	test eax, eax
	jz .loop

	mov eax, 0x04 ; write syscall
	mov ebx, 0x00 ; fd 0
	mov ecx, esp ; buf
	mov edx, 1 ; len

	int 0x80
	jmp .loop
.str:
	dw "a", 0xa
.end:

global loop2:function (loop2.end - loop2)
align 0x1000
loop2:
	mov eax, 0x04 ; write syscall
	mov ebx, 0x00 ; fd 0
	mov ecx, .str2 ; buf
	mov edx, 2 ; len

	int 0x80

	jmp loop2
	xchg bx, bx
	mov eax, 0x01
	mov ebx, 0x00
	int 0x80
	jmp loop2
.str2:
	dw "b", 0xa
.end:

resb 0x1000 ; buffer

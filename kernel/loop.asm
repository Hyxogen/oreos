; TODO remove
default rel

loop_handler:
	sub esp, 0x01

	mov esp, 'x'

	mov eax, 0x04
	mov ebx, 0x00
	mov ecx, esp
	mov edx, 0x01

	int 0x80

	mov eax, 0x01
	mov ebx, 0x10

	int 0x80
.halt:
	jmp .halt


global loop:function (loop.end - loop)
align 0x1000
loop:
	sub esp, 0x1 ; allocate 1 byte on the stack
.loop:
	mov eax, 0x30 ; signal syscall
	mov ebx, 11 ; SIGSEGV
	lea ecx, [loop_handler] ; handler

	int 0x80

	mov eax, 0x03 ; read syscall
	mov ebx, 0x00 ; fd 0
	mov ecx, esp ; buf
	mov edx, 1 ; len

	int 0x80

	test eax, eax
	jz .loop

	mov eax, [0]

	xchg bx, bx
	mov eax, 0x02 ; fork syscall
	int 0x80
	test eax, eax
	jnz .loop

	mov eax, 0x04 ; write syscall
	mov ebx, 0x00 ; fd 0
	mov ecx, esp ; buf
	mov edx, 1 ; len

	int 0x80

	mov eax, 0x01 ; exit syscall
	mov ebx, 0x00 ; status 0

	int 0x80
.halt:
	jmp .halt
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

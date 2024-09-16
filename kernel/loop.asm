; TODO remove
global loop:function (loop.end - loop)
loop:
	mov eax, 0x04 ; write syscall
	mov ebx, 0x00 ; fd 0
	mov ecx, .str ; buf
	mov edx, 2 ; len

	int 0x80
	jmp loop
.str:
	dw "a", 0xa
.end:

global loop2:function (loop2.end - loop2)
loop2:
	mov eax, 0x04 ; write syscall
	mov ebx, 0x00 ; fd 0
	mov ecx, .str2 ; buf
	mov edx, 2 ; len

	xchg bx, bx
	int 0x80
	jmp loop2
.str2:
	dw "b", 0xa
.end:

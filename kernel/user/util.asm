; write(int fd, const void *src, size_t nbytes)
global write:function (write.end - write)
write:
	push ebp
	mov ebp, esp

	push ebx

	mov eax, 0x04 ; write syscall
	mov ebx, [ebp + 8] ; fd
	mov ecx, [ebp + 12] ; src
	mov edx, [ebp + 16] ; nbytes

	int 0x80

	pop ebx
	pop ebp

	ret
.end:

; read(int fd, void *dest, size_t nbytes)
global read:function (read.end - read)
read:
	push ebp
	mov ebp, esp

	push ebx

	mov eax, 0x03 ; read syscall
	mov ebx, [ebp + 8] ; fd
	mov ecx, [ebp + 12] ; dest
	mov edx, [ebp + 16] ; nbytes

	int 0x80

	pop ebx
	pop ebp

	ret
.end:

; fork(void)
global fork:function (fork.end - fork)
fork:
	mov eax, 0x02 ; fork syscall

	int 0x80
	ret
.end:

; exit(int exit_status)
global exit:function (exit.end - exit)
exit:
	mov eax, 0x01 ; exit syscall
	mov ebx, [esp + 4] ; exit_status
 
	int 0x80

.halt:
	jmp .halt
.end:

; signal(int signum, void (*handler)(int))
global signal:function (signal.end - signal)
signal:
	push ebp
	mov ebp, esp

	push ebx

	mov eax, 0x30 ; signal syscall
	mov ebx, [ebp + 8] ; signum
	mov ecx, [ebp + 12] ; handler

	int 0x80

	pop ebx
	pop ebp
.end:

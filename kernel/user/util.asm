section .text
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

; getpid(void)
global getpid:function (getpid.end - getpid)
getpid:
	mov eax, 0x14 ; getpid syscall

	int 0x80

	ret
.end:

; kill(int pid, int sig)
global kill:function (kill.end - kill)
kill:
	push ebp
	mov ebp, esp

	push ebx

	mov eax, 0x25 ; kill syscall
	mov ebx, [ebp + 8] ; pid
	mov ecx, [ebp + 12] ; sig

	int 0x80

	pop ebx
	pop ebp

	ret
.end:

global __signal_trampoline:function (__signal_trampoline.end - __signal_trampoline)
__signal_trampoline:
	mov eax, [esp + 4] ; signum
	extern __signal_handlers
	mov eax, [__signal_handlers + 4 * eax]

	test eax, eax
	jz .finish ; TODO do default handler or something

	call eax

.finish:
	mov eax, 0x77 ; sigreturn syscall

	int 0x80

.halt:
	jmp .halt
.end:

; __signal(int signum, void (*handler)(int))
global __signal:function (__signal.end - __signal)
__signal:
	push ebp
	mov ebp, esp

	push ebx

	mov eax, 0x30 ; signal syscall
	mov ebx, [ebp + 8] ; signum
	mov ecx, [ebp + 12] ; handler

	int 0x80

	pop ebx
	pop ebp

	ret
.end:
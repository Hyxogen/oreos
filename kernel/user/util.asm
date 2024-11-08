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
	push ebp
	mov ebp, esp

	mov edx, [ebp + 8] ; signum
	extern __signal_handlers
	mov eax, [__signal_handlers + 4 * edx]

	test eax, eax
	jz .finish

	extern SIG_DFL
	lea ecx, SIG_DFL
	xchg [__signal_handlers + 4 * edx], ecx

	push ecx ; push previous handler
	push edx ; push signum

	; TODO temporarily set handler to default handler
	call eax

	pop edx ; pop signum
	pop ecx ; pop previous handler

	xchg [__signal_handlers + 4 * edx], ecx

.finish:
	pop ebp

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

; waitpid(int pid, int *wstatus, int options)
global waitpid:function (waitpid.end - waitpid)
waitpid:
	push ebp
	mov ebp, esp

	push ebx

	mov eax, 0x07 ; waitpid syscall
	mov ebx, [ebp + 8] ; pid
	mov ecx, [ebp + 12] ; wstatus
	mov edx, [ebp + 16] ; options

	int 0x80

	pop ebx
	pop ebp

	ret
.end:

; alarm(unsigned int seconds)
global alarm:function (alarm.end - alarm)
alarm:
	push ebp
	mov ebp, esp

	push ebx
	mov eax, 0x1b ; alarm syscall
	mov ebx, [ebp + 8] ; seconds

	int 0x80

	pop ebx
	pop ebp

	ret
.end:

; pause(void)
global pause:function (pause.end - pause)
pause:
	mov eax, 0x1d ; pause syscall

	int 0x80
	ret
.end:

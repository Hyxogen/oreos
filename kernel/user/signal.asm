section .text

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

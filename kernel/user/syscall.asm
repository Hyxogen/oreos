global __syscall0: function (__syscall0.end - __syscall0)
__syscall0:
	mov eax, [esp + 4] ; syscall number

	int 0x80
	ret
.end:

global __syscall1: function (__syscall1.end - __syscall1)
__syscall1:
	push ebp
	mov ebp, esp

	push ebx
	
	mov eax, [ebp + 8] ; syscall number
	mov ebx, [ebp + 12] ; first argument

	int 0x80

	pop ebx
	pop ebp

	ret
.end:

global __syscall2: function (__syscall2.end - __syscall2)
__syscall2:
	push ebp
	mov ebp, esp

	push ebx
	
	mov eax, [ebp + 8] ; syscall number
	mov ebx, [ebp + 12] ; first argument
	mov ecx, [ebp + 16] ; second argument

	int 0x80

	pop ebx
	pop ebp

	ret
.end:

global __syscall3: function (__syscall3.end - __syscall3)
__syscall3:
	push ebp
	mov ebp, esp

	push ebx
	
	mov eax, [ebp + 8] ; syscall number
	mov ebx, [ebp + 12] ; first argument
	mov ecx, [ebp + 16] ; second argument
	mov edx, [ebp + 20] ; third argument

	int 0x80

	pop ebx
	pop ebp

	ret
.end:

global __syscall4: function (__syscall4.end - __syscall4)
__syscall4:
	push ebp
	mov ebp, esp

	push ebx
	push esi
	
	mov eax, [ebp + 8] ; syscall number
	mov ebx, [ebp + 12] ; first argument
	mov ecx, [ebp + 16] ; second argument
	mov edx, [ebp + 20] ; third argument
	mov esi, [ebp + 24] ; fourth argument

	int 0x80

	pop esi
	pop ebx
	pop ebp

	ret
.end:

global __syscall5: function (__syscall5.end - __syscall5)
__syscall5:
	push ebp
	mov ebp, esp

	push ebx
	push esi
	push edi
	
	mov eax, [ebp + 8] ; syscall number
	mov ebx, [ebp + 12] ; first argument
	mov ecx, [ebp + 16] ; second argument
	mov edx, [ebp + 20] ; third argument
	mov esi, [ebp + 24] ; fourth argument
	mov edi, [ebp + 28] ; fifth argument

	int 0x80

	pop edi
	pop esi
	pop ebx
	pop ebp

	ret
.end:

global __syscall6: function (__syscall6.end - __syscall6)
__syscall6:
	push ebp
	mov ebp, esp

	push ebx
	push esi
	push edi
	push ebp
	
	mov eax, [ebp + 8] ; syscall number
	mov ebx, [ebp + 12] ; first argument
	mov ecx, [ebp + 16] ; second argument
	mov edx, [ebp + 20] ; third argument
	mov esi, [ebp + 24] ; fourth argument
	mov edi, [ebp + 28] ; fifth argument
	mov ebp, [ebp + 32] ; sixth argument
 
	int 0x80

	pop ebp
	pop edi
	pop esi
	pop ebx
	pop ebp

	ret
.end:

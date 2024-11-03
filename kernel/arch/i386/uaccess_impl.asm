section .user.text

; TODO rename this file
global __put_user1:function (__put_user1.end - __put_user1)
__put_user1:
	push ebp
	mov ebp, esp

	mov al, [esp + 12]
	mov ecx, [esp + 8]
	mov [ecx], al

	xor eax, eax
	pop ebp

	ret
.end:

global __get_user1:function (__get_user1.end - __get_user1)
__get_user1:
	push ebp
	mov ebp, esp

	mov eax, [esp + 12]
	mov al, [eax]

	mov ecx, [esp + 8]
	mov [ecx], al

	xor eax, eax
	pop ebp

	ret
.end:


global __user_memcpy:function (__user_memcpy.end - __user_memcpy)
__user_memcpy:
	push ebp
	mov ebp, esp

	push esi
	push edi

	mov ecx, [ebp + 16] ; nbytes
	mov esi, [ebp + 12] ; src
	mov edi, [ebp + 8] ; dest

	rep movsb 

	pop esi
	pop edi
	pop ebp

	xor eax, eax
	ret
.end:

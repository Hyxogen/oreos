section .text
; all interrupts will go through this common functions, which saves the
; registers etc.
_common_interrupt:
	; push registers on the stack
	pusha

	push dword ds

	push esp ; pass stack pointer as argument

	extern irq_callback
	call irq_callback
	mov esp, eax

	; load segments
	pop eax
	mov ds, ax
	mov es, ax
	mov fs, ax
	mov gs, ax
	; don't load ss, iret will do that for us

	; pop registers from the stack
	popa
	; free vector number and error code, see DEFINE_ISR macro
	add esp, 8
	iret


; arguments:
; $1 = interrupt vector (i.e. irq number)
; $2 = has error code
%macro DEFINE_ISR 2
align 16
vector_%1_handler:
	%if %2 == 0
	; push fake error code onto stack
	push dword 0
	%endif

	; push vector number onto stack
	push dword %1

	jmp _common_interrupt
%endmacro

%macro DEFINE_ISR_NOCODE 1
	DEFINE_ISR %1, 0
%endmacro

%macro DEFINE_ISR_CODE 1
	DEFINE_ISR %1, 1
%endmacro


global vector_0_handler
; TODO give magic values names with macros (which is annoying to redo in C...)
; perhaps switch to gcc assembler?
DEFINE_ISR_NOCODE	0
DEFINE_ISR_NOCODE 	1
DEFINE_ISR_NOCODE 	2
DEFINE_ISR_NOCODE 	3
DEFINE_ISR_NOCODE 	4
DEFINE_ISR_NOCODE 	5
DEFINE_ISR_NOCODE 	6
DEFINE_ISR_NOCODE 	7
DEFINE_ISR_CODE		8
DEFINE_ISR_NOCODE	9
DEFINE_ISR_CODE		10
DEFINE_ISR_CODE		11
DEFINE_ISR_CODE		12
DEFINE_ISR_CODE		13
DEFINE_ISR_CODE		14
DEFINE_ISR_NOCODE	15
DEFINE_ISR_NOCODE	16
DEFINE_ISR_CODE		17
DEFINE_ISR_NOCODE	18
DEFINE_ISR_NOCODE	19
DEFINE_ISR_NOCODE	20
DEFINE_ISR_CODE		21

%assign i 22
%rep 7
	DEFINE_ISR_NOCODE i
%assign i i+1
%endrep

DEFINE_ISR_CODE		29
DEFINE_ISR_CODE		30
DEFINE_ISR_NOCODE	31

%assign i 32
%rep 256-32
	DEFINE_ISR_NOCODE i
%assign i i+1
%endrep

.end

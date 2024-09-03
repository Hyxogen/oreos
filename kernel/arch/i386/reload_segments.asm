; void _reload_segments(code selector, data selector);
global _reload_segments:function (_reload_segments.end - _reload_segments)
_reload_segments:
	;TODO make sure that interrupts are disabled

	mov ax, [esp + 4]
	push word ax
	push dword .reload_cs

	; set cs
	jmp far [esp]
.reload_cs:
	add esp, 6

	mov ax, [esp + 8]
	mov ds, ax
	mov es, ax
	mov fs, ax
	mov gs, ax
	mov ss, ax
	ret
.end:

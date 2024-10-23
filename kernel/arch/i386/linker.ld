ENTRY(_start)

_kernel_addr = 0xC0000000;

SECTIONS
{
	/* skip over firmware stuff */
	. = 2M;

	_kernel_pstart = .;
	.multiboot.rodata : {
		*(.multiboot.rodata)
	}

	.multiboot.data : {
		*(.multiboot.data)
	}

	.multiboot.text : {
		*(.multiboot.text)
	}

	. += _kernel_addr;
	_kernel_vstart = .;
	_kernel_vro_start = .;
	.text ALIGN(4K) : AT (ADDR(.text) - _kernel_addr)
	{
		*(.multiboot)
		*(.text)
	}

	_user_text_start = .;
	.user.text ALIGN(4K) : AT(ADDR(.user.text) - _kernel_addr)
	{
		*(.user.text)
	}
	_user_text_end = .;

	.rodata ALIGN(4K) : AT (ADDR(.rodata) - _kernel_addr) 
	{
		*(.rodata)
	}
	_kernel_vro_end = .;

	.data ALIGN(4K) : AT (ADDR(.data) - _kernel_addr)
	{
		*(.data)
	}

	.bss ALIGN(4K) : AT (ADDR(.bss) - _kernel_addr)
	{
		*(COMMON)
		*(.bss)
	}

	_kernel_vend = .;
}

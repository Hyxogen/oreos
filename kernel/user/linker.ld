ENTRY(_start)

_start_addr = 0x00400000;

SECTIONS
{
	. = _start_addr;

	.text ALIGN(4K) :
	{
		*(.text)
	}

	.rodata ALIGN(4K) :
	{
		*(.rodata)
	}

	.data ALIGN(4K) :
	{
		*(.data)
	}

	.bss ALIGN(4K) :
	{
		*(COMMON)
		*(.bss)
	}
}

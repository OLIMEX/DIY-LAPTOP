OUTPUT_FORMAT("elf32-littlearm", "elf32-littlearm", "elf32-littlearm")
OUTPUT_ARCH(arm)
SECTIONS
{
	. = FES1ADDR;
	. = ALIGN(4);

	.head   :
	{
		main/fes_head.o	(.rodata)
	}
	.text :
	{
		main/fes1_entry.o (.text)
		*(.text)
	}
	. = ALIGN(16);
  	.rodata : { *(.rodata) }
	. = ALIGN(16);
  	.data : { *(.data) }

	. = ALIGN(4);
	.bss :
	{
	__bss_start = .;
		*(.bss)
	}
	. = ALIGN(4);
	__bss_end = .;

	_end = .;
}

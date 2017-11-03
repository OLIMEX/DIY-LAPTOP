OUTPUT_FORMAT("elf32-littlearm", "elf32-littlearm", "elf32-littlearm")
OUTPUT_ARCH(arm)
SECTIONS
{
	. = BOOT0ADDR;
	. = ALIGN(4);

	.head   :
	{
		main/boot0_head.o	(.rodata)
	}
        .hash   :
        {
                main/boot0_hash.o       (.data)
        }
        . = ALIGN(1);
	.text :
	{
		main/boot0_entry.o (.text)
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

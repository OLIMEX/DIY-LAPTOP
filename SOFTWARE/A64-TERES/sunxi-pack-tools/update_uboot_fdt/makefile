

objects = update_fdt.o


edit:$(objects)
	gcc -o update_uboot_fdt $(objects) -static
	rm -rf $(objects)


clean:
	rm -rf update_fdt $(objects)

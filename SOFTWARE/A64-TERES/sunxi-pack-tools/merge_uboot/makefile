objects =  merge_uboot.o
edit:$(objects)
	gcc -o merge_uboot $(objects) -static
	rm -rf $(objects)


clean:
	rm -rf merge_uboot $(objects)

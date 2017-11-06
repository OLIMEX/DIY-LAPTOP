

objects = check.o script.o update_boot0.o


edit:$(objects)
	gcc -o update_boot0 $(objects)
	rm -rf $(objects)


clean:
	rm -rf update_boot0 $(objects)
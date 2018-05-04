all:
	$(CC) teres1-debug.c -o teres1-debug
	$(CC) teres1-ledctrl.c -o teres1-ledctrl
clean:
	$(RM) teres1-debug
	$(RM) teres1-ledctrl
install: all
	install -m0755 teres1-debug /usr/local/sbin
	install -m0755 debug_switch.sh /usr/local/sbin
	install -m0755 teres1-ledctrl /usr/bin

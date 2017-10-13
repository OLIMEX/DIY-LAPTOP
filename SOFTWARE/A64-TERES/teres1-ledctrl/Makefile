all:
	$(CC) teres1-ledctrl.c -o teres1-ledctrl

clean:
	$(RM) teres1-ledctrl

install: all
	if ( test ! -d $(DESTDIR)/usr/bin ) ; then mkdir -p $(DESTDIR)/usr/bin ; fi
	install -m0755 teres1-ledctrl $(DESTDIR)/usr/bin

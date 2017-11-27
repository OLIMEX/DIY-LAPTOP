all:
	$(CC) teres1-audioselect.c -o teres1-audioselect

clean:
	$(RM) teres1-audioselect

install: all
	if ( test ! -d $(DESTDIR)/usr/bin ) ; then mkdir -p $(DESTDIR)/usr/bin ; fi
	install -m0755 teres1-audioselect $(DESTDIR)/usr/bin

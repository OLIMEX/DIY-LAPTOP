all:
	$(CC) teres1-audioselect.c -o teres1-audioselect

clean:
	$(RM) teres1-audioselect

install: all
	install -m0755 teres1-audioselect /usr/bin
	install -m0644 teres1-audioselect.service /etc/systemd/system/teres1-audioselect.service && \
		systemctl enable teres1-audioselect.service && \
		systemctl start teres1-audioselect.service

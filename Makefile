# See LICENSE file for copyright and license details.
# slstatus - suckless status monitor
.POSIX:

include config.mk

all: slstatus

slstatus: slstatus.c config.h config.mk
	$(CC) -o $@ $(CFLAGS) $(CPPFLAGS) $(LDFLAGS) slstatus.c $(LDLIBS)

config.h:
	cp config.def.h $@

clean:
	rm -f slstatus

dist:
	rm -rf "slstatus-$(VERSION)"
	mkdir -p "slstatus-$(VERSION)"
	cp -R arg.h config.def.h config.mk LICENSE Makefile README slstatus.1 \
		slstatus.c slstatus.png "slstatus-$(VERSION)"
	tar -cf - "slstatus-$(VERSION)" | gzip -c > "slstatus-$(VERSION).tar.gz"
	rm -rf "slstatus-$(VERSION)"

install: all
	mkdir -p "$(DESTDIR)$(PREFIX)/bin"
	cp -f slstatus "$(DESTDIR)$(PREFIX)/bin"
	chmod 755 "$(DESTDIR)$(PREFIX)/bin/slstatus"
	mkdir -p "$(DESTDIR)$(MANPREFIX)/man1"
	cp -f slstatus.1 "$(DESTDIR)$(MANPREFIX)/man1"
	chmod 644 "$(DESTDIR)$(MANPREFIX)/man1/slstatus.1"

uninstall:
	rm -f "$(DESTDIR)$(PREFIX)/bin/slstatus"
	rm -f "$(DESTDIR)$(MANPREFIX)/man1/slstatus.1"

# See LICENSE file for copyright and license details.

include config.mk

all: slstatus

slstatus: slstatus.c config.h config.mk
	${CC} ${CFLAGS} -o $@ slstatus.c ${LDFLAGS}

config.h:
	cp config.def.h $@

clean:
	rm -f slstatus

install: all
	mkdir -p ${DESTDIR}${PREFIX}/bin
	cp -f slstatus ${DESTDIR}${PREFIX}/bin
	chmod 755 ${DESTDIR}${PREFIX}/bin/slstatus
	mkdir -p ${DESTDIR}${MANPREFIX}/man1
	cp -f slstatus.1 ${DESTDIR}${MANPREFIX}/man1
	chmod 644 ${DESTDIR}${MANPREFIX}/man1/slstatus.1

uninstall:
	rm -f ${DESTDIR}${PREFIX}/bin/slstatus
	rm -f ${DESTDIR}${MANPREFIX}/man1/slstatus.1

.PHONY: all clean install uninstall

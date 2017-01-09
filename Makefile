# See LICENSE file for copyright and license details.

include config.mk

NAME=slstatus

all: ${NAME}

${NAME}: config.h
	${CC} ${CFLAGS} -o $@ ${NAME}.c ${LDFLAGS}

config.h:
	cp config.def.h $@

clean:
	rm -f ${NAME}

install: all
	mkdir -p ${DESTDIR}${PREFIX}/bin
	cp -f ${NAME} ${DESTDIR}${PREFIX}/bin
	chmod 755 ${DESTDIR}${PREFIX}/bin/${NAME}
	mkdir -p ${DESTDIR}${MANPREFIX}/man1
	cp -f ${NAME}.1 ${DESTDIR}${MANPREFIX}/man1
	chmod 644 ${DESTDIR}${MANPREFIX}/man1/${NAME}.1

uninstall:
	rm -f ${DESTDIR}${PREFIX}/bin/${NAME}
	rm -f ${DESTDIR}${MANPREFIX}/man1/${NAME}.1

.PHONY: all clean install uninstall

-include config.mk

VERSION = 0.6

CFLAGS_STD ?= -std=c99 -D_POSIX_C_SOURCE=200809L -D_XOPEN_SOURCE=700 -DNDEBUG
CFLAGS_STD += -DVERSION=\"${VERSION}\"

LDFLAGS_STD ?= -lc -lutil

STRIP ?= strip
INSTALL ?= install

PREFIX ?= /usr
SHAREDIR ?= ${PREFIX}/share

SRC = abduco-sb.c

all: abduco-sb

config.h:
	cp config.def.h config.h

config.mk:
	@touch $@

abduco-sb: config.h config.mk *.c
	${CC} ${CFLAGS} ${CFLAGS_STD} ${CFLAGS_AUTO} ${CFLAGS_EXTRA} ${SRC} ${LDFLAGS} ${LDFLAGS_STD} ${LDFLAGS_AUTO} -o abduco-sb

debug: clean
	make CFLAGS_EXTRA='${CFLAGS_DEBUG}'

clean:
	@echo cleaning
	@rm -f abduco-sb abduco-sb-*.tar.gz

dist: clean
	@echo creating dist tarball
	@git archive --prefix=abduco-sb-${VERSION}/ -o abduco-sb-${VERSION}.tar.gz HEAD

installdirs:
	@${INSTALL} -d ${DESTDIR}${PREFIX}/bin \
		${DESTDIR}${MANPREFIX}/man1

install: abduco-sb installdirs
	@echo installing executable file to ${DESTDIR}${PREFIX}/bin
	@${INSTALL} -m 0755 abduco-sb ${DESTDIR}${PREFIX}/bin
	@echo installing manual page to ${DESTDIR}${MANPREFIX}/man1
	@mkdir -p ${DESTDIR}${MANPREFIX}/man1
	@sed "s/VERSION/${VERSION}/g" < abduco.1 > ${DESTDIR}${MANPREFIX}/man1/abduco-sb.1
	@chmod 644 ${DESTDIR}${MANPREFIX}/man1/abduco-sb.1

install-strip: install
	${STRIP} ${DESTDIR}${PREFIX}/bin/abduco-sb

install-completion:
	@echo installing zsh completion file to ${DESTDIR}${SHAREDIR}/zsh/site-functions
	@install -Dm644 contrib/abduco.zsh ${DESTDIR}${SHAREDIR}/zsh/site-functions/_abduco-sb

uninstall:
	@echo removing executable file from ${DESTDIR}${PREFIX}/bin
	@rm -f ${DESTDIR}${PREFIX}/bin/abduco-sb
	@echo removing manual page from ${DESTDIR}${MANPREFIX}/man1
	@rm -f ${DESTDIR}${MANPREFIX}/man1/abduco-sb.1
	@echo removing zsh completion file from ${DESTDIR}${SHAREDIR}/zsh/site-functions
	@rm -f ${DESTDIR}${SHAREDIR}/zsh/site-functions/_abduco-sb

.PHONY: all clean dist install installdirs install-strip install-completion uninstall debug

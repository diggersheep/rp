PACKAGE = 'rp-tests'
VERSION = '0.0.1'

PREFIX := /usr/local
BINDIR := $(PREFIX)/bin
LIBDIR := $(PREFIX)/lib
SHAREDIR := $(PREFIX)/share
INCLUDEDIR := $(PREFIX)/include

CC := cc
AR := ar
RANLIB := ranlib
CFLAGS := -std=gnu11 -Wall -Wextra -Werror
LDFLAGS := 

Q := @

all: server client peer

server: server.o ../net.o ../debug.o 
	@echo '[01;32m  LD >    [01;37mserver[00m'
	$(Q)$(CC) -o server $(LDFLAGS) server.o ../net.o ../debug.o 

server.install: server
	@echo '[01;31m  IN >    [01;37m$(BINDIR)/server[00m'
	$(Q)mkdir -p '$(DESTDIR)$(BINDIR)'
	$(Q)install -m0755 server $(DESTDIR)$(BINDIR)/server

server.clean:  server.o.clean ../net.o.clean ../debug.o.clean
	@echo '[01;37m  RM >    [01;37mserver[00m'
	$(Q)rm -f server

server.uninstall:
	@echo '[01;37m  RM >    [01;37m$(BINDIR)/server[00m'
	$(Q)rm -f '$(DESTDIR)$(BINDIR)/server'

client: client.o ../net.o ../debug.o 
	@echo '[01;32m  LD >    [01;37mclient[00m'
	$(Q)$(CC) -o client $(LDFLAGS) client.o ../net.o ../debug.o 

client.install: client
	@echo '[01;31m  IN >    [01;37m$(BINDIR)/client[00m'
	$(Q)mkdir -p '$(DESTDIR)$(BINDIR)'
	$(Q)install -m0755 client $(DESTDIR)$(BINDIR)/client

client.clean:  client.o.clean ../net.o.clean ../debug.o.clean
	@echo '[01;37m  RM >    [01;37mclient[00m'
	$(Q)rm -f client

client.uninstall:
	@echo '[01;37m  RM >    [01;37m$(BINDIR)/client[00m'
	$(Q)rm -f '$(DESTDIR)$(BINDIR)/client'

peer: peer.o ../net.o ../debug.o 
	@echo '[01;32m  LD >    [01;37mpeer[00m'
	$(Q)$(CC) -o peer $(LDFLAGS) peer.o ../net.o ../debug.o 

peer.install: peer
	@echo '[01;31m  IN >    [01;37m$(BINDIR)/peer[00m'
	$(Q)mkdir -p '$(DESTDIR)$(BINDIR)'
	$(Q)install -m0755 peer $(DESTDIR)$(BINDIR)/peer

peer.clean:  peer.o.clean ../net.o.clean ../debug.o.clean
	@echo '[01;37m  RM >    [01;37mpeer[00m'
	$(Q)rm -f peer

peer.uninstall:
	@echo '[01;37m  RM >    [01;37m$(BINDIR)/peer[00m'
	$(Q)rm -f '$(DESTDIR)$(BINDIR)/peer'

server.o: server.c ./../net.h
	@echo '[01;34m  CC >    [01;37mserver.o[00m'
	$(Q)$(CC) $(CFLAGS)  -c server.c   -o server.o

server.o.install:

server.o.clean:
	@echo '[01;37m  RM >    [01;37mserver.o[00m'
	$(Q)rm -f server.o

server.o.uninstall:

../net.o: ../net.c ../net.h ../vec/vec.h ../debug.h
	@echo '[01;34m  CC >    [01;37m../net.o[00m'
	$(Q)$(CC) $(CFLAGS)  -c ../net.c   -o ../net.o

../net.o.install:

../net.o.clean:
	@echo '[01;37m  RM >    [01;37m../net.o[00m'
	$(Q)rm -f ../net.o

../net.o.uninstall:

../debug.o: ../debug.c
	@echo '[01;34m  CC >    [01;37m../debug.o[00m'
	$(Q)$(CC) $(CFLAGS)  -c ../debug.c   -o ../debug.o

../debug.o.install:

../debug.o.clean:
	@echo '[01;37m  RM >    [01;37m../debug.o[00m'
	$(Q)rm -f ../debug.o

../debug.o.uninstall:

client.o: client.c ./../net.h
	@echo '[01;34m  CC >    [01;37mclient.o[00m'
	$(Q)$(CC) $(CFLAGS)  -c client.c   -o client.o

client.o.install:

client.o.clean:
	@echo '[01;37m  RM >    [01;37mclient.o[00m'
	$(Q)rm -f client.o

client.o.uninstall:

peer.o: peer.c ./../net.h
	@echo '[01;34m  CC >    [01;37mpeer.o[00m'
	$(Q)$(CC) $(CFLAGS)  -c peer.c   -o peer.o

peer.o.install:

peer.o.clean:
	@echo '[01;37m  RM >    [01;37mpeer.o[00m'
	$(Q)rm -f peer.o

peer.o.uninstall:

$(DESTDIR)$(PREFIX):
	@echo '[01;35m  DIR >   [01;37m$(PREFIX)[00m'
	$(Q)mkdir -p $(DESTDIR)$(PREFIX)
$(DESTDIR)$(BINDIR):
	@echo '[01;35m  DIR >   [01;37m$(BINDIR)[00m'
	$(Q)mkdir -p $(DESTDIR)$(BINDIR)
$(DESTDIR)$(LIBDIR):
	@echo '[01;35m  DIR >   [01;37m$(LIBDIR)[00m'
	$(Q)mkdir -p $(DESTDIR)$(LIBDIR)
$(DESTDIR)$(SHAREDIR):
	@echo '[01;35m  DIR >   [01;37m$(SHAREDIR)[00m'
	$(Q)mkdir -p $(DESTDIR)$(SHAREDIR)
$(DESTDIR)$(INCLUDEDIR):
	@echo '[01;35m  DIR >   [01;37m$(INCLUDEDIR)[00m'
	$(Q)mkdir -p $(DESTDIR)$(INCLUDEDIR)
install: subdirs.install server.install client.install peer.install server.o.install ../net.o.install ../debug.o.install client.o.install ../net.o.install ../debug.o.install peer.o.install ../net.o.install ../debug.o.install
	@:

subdirs.install:

uninstall: subdirs.uninstall server.uninstall client.uninstall peer.uninstall server.o.uninstall ../net.o.uninstall ../debug.o.uninstall client.o.uninstall ../net.o.uninstall ../debug.o.uninstall peer.o.uninstall ../net.o.uninstall ../debug.o.uninstall
	@:

subdirs.uninstall:

test: all subdirs subdirs.test
	@:

subdirs.test:

clean: server.clean client.clean peer.clean server.o.clean ../net.o.clean ../debug.o.clean client.o.clean ../net.o.clean ../debug.o.clean peer.o.clean ../net.o.clean ../debug.o.clean

distclean: clean

dist: dist-gz dist-xz dist-bz2
	$(Q)rm -- $(PACKAGE)-$(VERSION)

distdir:
	$(Q)rm -rf -- $(PACKAGE)-$(VERSION)
	$(Q)ln -s -- . $(PACKAGE)-$(VERSION)

dist-gz: $(PACKAGE)-$(VERSION).tar.gz
$(PACKAGE)-$(VERSION).tar.gz: distdir
	@echo '[01;33m  TAR >   [01;37m$(PACKAGE)-$(VERSION).tar.gz[00m'
	$(Q)tar czf $(PACKAGE)-$(VERSION).tar.gz \
		$(PACKAGE)-$(VERSION)/peer.c \
		$(PACKAGE)-$(VERSION)/server.c \
		$(PACKAGE)-$(VERSION)/client.c \
		$(PACKAGE)-$(VERSION)/../net.c \
		$(PACKAGE)-$(VERSION)/../debug.c \
		$(PACKAGE)-$(VERSION)/../net.h \
		$(PACKAGE)-$(VERSION)/../debug.h

dist-xz: $(PACKAGE)-$(VERSION).tar.xz
$(PACKAGE)-$(VERSION).tar.xz: distdir
	@echo '[01;33m  TAR >   [01;37m$(PACKAGE)-$(VERSION).tar.xz[00m'
	$(Q)tar cJf $(PACKAGE)-$(VERSION).tar.xz \
		$(PACKAGE)-$(VERSION)/peer.c \
		$(PACKAGE)-$(VERSION)/server.c \
		$(PACKAGE)-$(VERSION)/client.c \
		$(PACKAGE)-$(VERSION)/../net.c \
		$(PACKAGE)-$(VERSION)/../debug.c \
		$(PACKAGE)-$(VERSION)/../net.h \
		$(PACKAGE)-$(VERSION)/../debug.h

dist-bz2: $(PACKAGE)-$(VERSION).tar.bz2
$(PACKAGE)-$(VERSION).tar.bz2: distdir
	@echo '[01;33m  TAR >   [01;37m$(PACKAGE)-$(VERSION).tar.bz2[00m'
	$(Q)tar cjf $(PACKAGE)-$(VERSION).tar.bz2 \
		$(PACKAGE)-$(VERSION)/peer.c \
		$(PACKAGE)-$(VERSION)/server.c \
		$(PACKAGE)-$(VERSION)/client.c \
		$(PACKAGE)-$(VERSION)/../net.c \
		$(PACKAGE)-$(VERSION)/../debug.c \
		$(PACKAGE)-$(VERSION)/../net.h \
		$(PACKAGE)-$(VERSION)/../debug.h

help:
	@echo '[01;37m :: rp-tests-0.0.1[00m'
	@echo ''
	@echo '[01;37mGeneric targets:[00m'
	@echo '[00m    - [01;32mhelp          [37m Prints this help message.[00m'
	@echo '[00m    - [01;32mall           [37m Builds all targets.[00m'
	@echo '[00m    - [01;32mdist          [37m Creates tarballs of the files of the project.[00m'
	@echo '[00m    - [01;32minstall       [37m Installs the project.[00m'
	@echo '[00m    - [01;32mclean         [37m Removes compiled files.[00m'
	@echo '[00m    - [01;32muninstall     [37m Deinstalls the project.[00m'
	@echo ''
	@echo '[01;37mCLI-modifiable variables:[00m'
	@echo '    - [01;34mCC            [37m ${CC}[00m'
	@echo '    - [01;34mCFLAGS        [37m ${CFLAGS}[00m'
	@echo '    - [01;34mLDFLAGS       [37m ${LDFLAGS}[00m'
	@echo '    - [01;34mDESTDIR       [37m ${DESTDIR}[00m'
	@echo '    - [01;34mPREFIX        [37m ${PREFIX}[00m'
	@echo '    - [01;34mBINDIR        [37m ${BINDIR}[00m'
	@echo '    - [01;34mLIBDIR        [37m ${LIBDIR}[00m'
	@echo '    - [01;34mSHAREDIR      [37m ${SHAREDIR}[00m'
	@echo '    - [01;34mINCLUDEDIR    [37m ${INCLUDEDIR}[00m'
	@echo ''
	@echo '[01;37mProject targets: [00m'
	@echo '    - [01;33mserver        [37m binary[00m'
	@echo '    - [01;33mclient        [37m binary[00m'
	@echo '    - [01;33mpeer          [37m binary[00m'
	@echo ''
	@echo '[01;37mMakefile options:[00m'
	@echo '    - gnu:           true'
	@echo '    - colors:        true'
	@echo ''
	@echo '[01;37mRebuild the Makefile with:[00m'
	@echo '    zsh ./build.zsh -c -g'
.PHONY: all subdirs clean distclean dist install uninstall help


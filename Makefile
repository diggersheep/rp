PACKAGE = 'test'
VERSION = '0.0.1'

PREFIX := /usr/local
BINDIR := $(PREFIX)/bin
LIBDIR := $(PREFIX)/lib
SHAREDIR := $(PREFIX)/share
INCLUDEDIR := $(PREFIX)/include

CC := clang
AR := ar
RANLIB := ranlib
CFLAGS := -std=gnu11 -Wall -Wextra -g -O2
LDFLAGS := 

Q := @

all: hash-file

hash-file: hash-file.o hash.o sha256/sha256.o vec/vec.o 
	@echo '[01;32m  [LD]    [01;37mhash-file[00m'
	$(Q)$(CC) -o hash-file $(LDFLAGS) hash-file.o hash.o sha256/sha256.o vec/vec.o 

hash-file.install: hash-file
	@echo '[01;31m  [IN]    [01;37m$(BINDIR)/hash-file[00m'
	$(Q)mkdir -p '$(DESTDIR)$(BINDIR)'
	$(Q)install -m0755 hash-file $(DESTDIR)$(BINDIR)/hash-file

hash-file.clean:  hash-file.o.clean hash.o.clean sha256/sha256.o.clean vec/vec.o.clean
	@echo '[01;37m  [RM]    [01;37mhash-file[00m'
	$(Q)rm -f hash-file

hash-file.uninstall:
	@echo '[01;37m  [RM]    [01;37m$(BINDIR)/hash-file[00m'
	$(Q)rm -f '$(DESTDIR)$(BINDIR)/hash-file'

hash-file.o: hash-file.c ./vec/vec.h ./sha256/sha256.h ./common.h ./hash.h
	@echo '[01;34m  [CC]    [01;37mhash-file.o[00m'
	$(Q)$(CC) $(CFLAGS)  -c hash-file.c   -o hash-file.o

hash-file.o.install:

hash-file.o.clean:
	@echo '[01;37m  [RM]    [01;37mhash-file.o[00m'
	$(Q)rm -f hash-file.o

hash-file.o.uninstall:

hash.o: hash.c ./vec/vec.h ./sha256/sha256.h ./common.h
	@echo '[01;34m  [CC]    [01;37mhash.o[00m'
	$(Q)$(CC) $(CFLAGS)  -c hash.c   -o hash.o

hash.o.install:

hash.o.clean:
	@echo '[01;37m  [RM]    [01;37mhash.o[00m'
	$(Q)rm -f hash.o

hash.o.uninstall:

sha256/sha256.o: sha256/sha256.c sha256/rotate-bits.h sha256/sha256.h
	@echo '[01;34m  [CC]    [01;37msha256/sha256.o[00m'
	$(Q)$(CC) $(CFLAGS)  -c sha256/sha256.c   -o sha256/sha256.o

sha256/sha256.o.install:

sha256/sha256.o.clean:
	@echo '[01;37m  [RM]    [01;37msha256/sha256.o[00m'
	$(Q)rm -f sha256/sha256.o

sha256/sha256.o.uninstall:

vec/vec.o: vec/vec.c vec/vec.h
	@echo '[01;34m  [CC]    [01;37mvec/vec.o[00m'
	$(Q)$(CC) $(CFLAGS)  -c vec/vec.c   -o vec/vec.o

vec/vec.o.install:

vec/vec.o.clean:
	@echo '[01;37m  [RM]    [01;37mvec/vec.o[00m'
	$(Q)rm -f vec/vec.o

vec/vec.o.uninstall:

$(DESTDIR)$(PREFIX):
	@echo '[01;35m  [DIR]   [01;37m$(PREFIX)[00m'
	$(Q)mkdir -p $(DESTDIR)$(PREFIX)
$(DESTDIR)$(BINDIR):
	@echo '[01;35m  [DIR]   [01;37m$(BINDIR)[00m'
	$(Q)mkdir -p $(DESTDIR)$(BINDIR)
$(DESTDIR)$(LIBDIR):
	@echo '[01;35m  [DIR]   [01;37m$(LIBDIR)[00m'
	$(Q)mkdir -p $(DESTDIR)$(LIBDIR)
$(DESTDIR)$(SHAREDIR):
	@echo '[01;35m  [DIR]   [01;37m$(SHAREDIR)[00m'
	$(Q)mkdir -p $(DESTDIR)$(SHAREDIR)
$(DESTDIR)$(INCLUDEDIR):
	@echo '[01;35m  [DIR]   [01;37m$(INCLUDEDIR)[00m'
	$(Q)mkdir -p $(DESTDIR)$(INCLUDEDIR)
install: subdirs.install hash-file.install hash-file.o.install hash.o.install sha256/sha256.o.install vec/vec.o.install
	@:

subdirs.install:

uninstall: subdirs.uninstall hash-file.uninstall hash-file.o.uninstall hash.o.uninstall sha256/sha256.o.uninstall vec/vec.o.uninstall
	@:

subdirs.uninstall:

test: all subdirs subdirs.test
	@:

subdirs.test:

clean: hash-file.clean hash-file.o.clean hash.o.clean sha256/sha256.o.clean vec/vec.o.clean

distclean: clean

dist: dist-gz dist-xz dist-bz2
	$(Q)rm -- $(PACKAGE)-$(VERSION)

distdir:
	$(Q)rm -rf -- $(PACKAGE)-$(VERSION)
	$(Q)ln -s -- . $(PACKAGE)-$(VERSION)

dist-gz: $(PACKAGE)-$(VERSION).tar.gz
$(PACKAGE)-$(VERSION).tar.gz: distdir
	@echo '[01;33m  [TAR]   [01;37m$(PACKAGE)-$(VERSION).tar.gz[00m'
	$(Q)tar czf $(PACKAGE)-$(VERSION).tar.gz \
		$(PACKAGE)-$(VERSION)/hash-file.c \
		$(PACKAGE)-$(VERSION)/hash.c \
		$(PACKAGE)-$(VERSION)/sha256/sha256.c \
		$(PACKAGE)-$(VERSION)/vec/vec.c \
		$(PACKAGE)-$(VERSION)/hash.h \
		$(PACKAGE)-$(VERSION)/sha256/sha256.h \
		$(PACKAGE)-$(VERSION)/vec/vec.h

dist-xz: $(PACKAGE)-$(VERSION).tar.xz
$(PACKAGE)-$(VERSION).tar.xz: distdir
	@echo '[01;33m  [TAR]   [01;37m$(PACKAGE)-$(VERSION).tar.xz[00m'
	$(Q)tar cJf $(PACKAGE)-$(VERSION).tar.xz \
		$(PACKAGE)-$(VERSION)/hash-file.c \
		$(PACKAGE)-$(VERSION)/hash.c \
		$(PACKAGE)-$(VERSION)/sha256/sha256.c \
		$(PACKAGE)-$(VERSION)/vec/vec.c \
		$(PACKAGE)-$(VERSION)/hash.h \
		$(PACKAGE)-$(VERSION)/sha256/sha256.h \
		$(PACKAGE)-$(VERSION)/vec/vec.h

dist-bz2: $(PACKAGE)-$(VERSION).tar.bz2
$(PACKAGE)-$(VERSION).tar.bz2: distdir
	@echo '[01;33m  [TAR]   [01;37m$(PACKAGE)-$(VERSION).tar.bz2[00m'
	$(Q)tar cjf $(PACKAGE)-$(VERSION).tar.bz2 \
		$(PACKAGE)-$(VERSION)/hash-file.c \
		$(PACKAGE)-$(VERSION)/hash.c \
		$(PACKAGE)-$(VERSION)/sha256/sha256.c \
		$(PACKAGE)-$(VERSION)/vec/vec.c \
		$(PACKAGE)-$(VERSION)/hash.h \
		$(PACKAGE)-$(VERSION)/sha256/sha256.h \
		$(PACKAGE)-$(VERSION)/vec/vec.h

help:
	@echo '[01;37m :: test-0.0.1[00m'
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
	@echo '    - [01;33mhash-file     [37m binary[00m'
	@echo ''
	@echo '[01;37mMakefile options:[00m'
	@echo '    - gnu:           true'
	@echo '    - colors:        true'
	@echo ''
	@echo '[01;37mRebuild the Makefile with:[00m'
	@echo '    zsh ./build.zsh -c -g'
.PHONY: all subdirs clean distclean dist install uninstall help


.PHONY: all install clean dist dist-all dist-bzip2 dist-gzip dist-zip

DIST_DIR=dist

# Utilities
INSTALL = install
INSTALL_PROGRAM = $(INSTALL)
INSTALL_DATA = $(INSTALL) -m 644

# Flags
CFLAGS = -Wall -fpic
LDFLAGS = -shared

# Directories
LUA_INC_DIR ?= /usr/include/lua5.1

INSTALL_DIR_LUA ?= /usr/share/lua/5.1
INSTALL_DIR_BIN ?= /usr/lib/lua/5.1

# Files
LIB = core.so

all:
	$(CC) $(CFLAGS) -c -Iinclude -I$(LUA_INC_DIR) src/*.c
	$(CC) $(LDFLAGS) -o $(LIB) *.o -levent

dist dist-all: distdir dist-bzip2 dist-gzip dist-zip

distdir:
	mkdir -p $(DIST_DIR)

VERSION=luaevent-$(shell git describe --abbrev=4 HEAD 2>/dev/null)
dist-bzip2: distdir
	git archive --format=tar --prefix=$(VERSION)/ HEAD | bzip2 -9v > $(DIST_DIR)/$(VERSION).tar.bz2
dist-gzip: distdir
	git archive --format=tar --prefix=$(VERSION)/ HEAD | gzip -9v > $(DIST_DIR)/$(VERSION).tar.gz
dist-zip: distdir
	git archive --format=zip --prefix=$(VERSION)/ HEAD > $(DIST_DIR)/$(VERSION).zip

install: all
	mkdir -p $(DESTDIR)$(INSTALL_DIR_LUA)
	$(INSTALL_DATA) lua/luaevent.lua $(DESTDIR)$(INSTALL_DIR_LUA)/luaevent.lua
	mkdir -p $(DESTDIR)$(INSTALL_DIR_BIN)/luaevent/
	$(INSTALL_PROGRAM) $(LIB) $(DESTDIR)$(INSTALL_DIR_BIN)/luaevent/$(LIB)

clean:
	rm -f *.so
	rm -f *.o


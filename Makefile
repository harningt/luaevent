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

install: all
	mkdir -p $(DESTDIR)$(INSTALL_DIR_LUA)
	$(INSTALL_DATA) lua/luaevent.lua $(DESTDIR)$(INSTALL_DIR_LUA)/luaevent.lua
	mkdir -p $(DESTDIR)$(INSTALL_DIR_BIN)/luaevent/
	$(INSTALL_PROGRAM) $(LIB) $(DESTDIR)$(INSTALL_DIR_BIN)/luaevent/$(LIB)

clean:
	rm -f *.so
	rm -f *.o


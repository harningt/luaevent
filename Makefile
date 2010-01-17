LUA_INC_DIR=/usr/include/lua5.1
LUA_LIB_DIR=/usr/lib

INSTALL_DIR_LUA=/usr/share/lua/5.1
INSTALL_DIR_BIN=/usr/lib/lua/5.1


all:
	gcc -g -O0 -c -Wall -fpic -Iinclude -I$(LUA_INC_DIR) src/*.c
	gcc -g -shared -fpic -o core.so *.o -L$(LUA_LIB_DIR) -llua5.1 -levent

install:
	install -d $(INSTALL_DIR_LUA)
	install lua/luaevent.lua $(INSTALL_DIR_LUA)/
	install -d $(INSTALL_DIR_BIN)/luaevent
	install core.so $(INSTALL_DIR_BIN)/luaevent/core.so

clean:
	rm *.so
	rm *.o


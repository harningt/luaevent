/* LuaEvent - Copyright (C) 2007 Thomas Harning <harningt@gmail.com>
 * Licensed as LGPL - See doc/COPYING for details */
#ifndef LUAEVENT_H
#define LUAEVENT_H

#include <lua.h>
#include <sys/types.h>
#include <sys/time.h>
#include <event.h>

typedef struct {
	struct event_base* base;
	lua_State* loop_L;
} le_base;

typedef struct {
	struct event ev;
	le_base* base;
	int callbackRef;
} le_callback;

int luaopen_luaevent(lua_State* L);

#endif

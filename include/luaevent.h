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

le_base* event_base_get(lua_State* L, int idx);
void load_timeval(double time, struct timeval *tv);

int luaopen_luaevent(lua_State* L);

#endif

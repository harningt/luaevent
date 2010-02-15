/* LuaEvent - Copyright (C) 2007 Thomas Harning <harningt@gmail.com>
 * Licensed as LGPL - See doc/COPYING for details */
#ifndef LUAEVENT_H
#define LUAEVENT_H

#include <lua.h>
#include <sys/types.h>
#ifdef _WIN32
#include <winsock2.h>
#else
#include <sys/time.h>
#endif
#include <event.h>

typedef struct {
	struct event_base* base;
	lua_State* loop_L;
} le_base;

le_base* event_base_get(lua_State* L, int idx);
void load_timeval(double time, struct timeval *tv);
int getSocketFd(lua_State* L, int idx);

int luaopen_luaevent(lua_State* L);

#endif

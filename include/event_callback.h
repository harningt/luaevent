/* LuaEvent - Copyright (C) 2007 Thomas Harning <harningt@gmail.com>
 * Licensed as LGPL - See doc/COPYING for details */
#ifndef EVENT_CALLBACK
#define EVENT_CALLBACK

#include "luaevent.h"
#include <lua.h>
#include <sys/types.h>
#include <sys/time.h>
#include <event.h>

typedef struct {
	struct event ev;
	le_base* base;
	int callbackRef;
} le_callback;

int event_callback_register(lua_State* L);

le_callback* event_callback_push(lua_State* L, int baseIdx, int callbackIdx);

void luaevent_callback(int fd, short event, void* p);

#endif

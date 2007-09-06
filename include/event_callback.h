/* LuaEvent - Copyright (C) 2007 Thomas Harning <harningt@gmail.com>
 * Licensed as LGPL - See doc/COPYING for details */
#ifndef EVENT_CALLBACK
#define EVENT_CALLBACK

#include "luaevent.h"
#include <lua.h>
#include <sys/types.h>
#include <sys/time.h>
#include <event.h>

#define EVENT_CALLBACK_ARG_MT "EVENT_CALLBACK_ARG_MT"

typedef struct {
	struct event ev;
	le_base* base;
	int callbackRef;
} le_callback;

int event_callback_register(lua_State* L);

void luaevent_callback(int fd, short event, void* p);

#endif

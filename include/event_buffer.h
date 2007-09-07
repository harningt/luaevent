/* LuaEvent - Copyright (C) 2007 Thomas Harning <harningt@gmail.com>
 * Licensed as LGPL - See doc/COPYING for details */
#ifndef EVENT_BUFFER_H
#define EVENT_BUFFER_H

#include "luaevent.h"
#include <lua.h>
#include <sys/types.h>
#include <sys/time.h>
#include <event.h>

typedef struct {
	struct evbuffer* buffer;
} le_buffer;

int event_buffer_register(lua_State* L);

#endif

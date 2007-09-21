/* LuaEvent - Copyright (C) 2007 Thomas Harning <harningt@gmail.com>
 * Licensed as LGPL - See doc/COPYING for details */
 #ifndef BUFFER_EVENT_H
 #define BUFFER_EVENT_H
 
#include "luaevent.h"
#include <lua.h>
#include <sys/types.h>
#include <sys/time.h>
#include <event.h>

typedef struct {
	struct bufferevent* ev;
	le_base* base;
} le_bufferevent;

int buffer_event_register(lua_State* L);
int is_buffer_event(lua_State* L, int idx);
le_bufferevent* buffer_event_check(lua_State* L, int idx);

#endif

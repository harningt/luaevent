/* LuaEvent - Copyright (C) 2007 Thomas Harning <harningt@gmail.com>
 * Licensed as LGPL - See doc/COPYING for details */

#include "event_buffer.h"
#include <lauxlib.h>

#define EVENT_BUFFER_MT "EVENT_BUFFER_MT"

#define BUFFER_ADD_CHECK_INPUT_FIRST 1

static le_buffer* event_buffer_get(lua_State* L, int idx) {
	return (le_buffer*)luaL_checkudata(L, idx, EVENT_BUFFER_MT);
}
static int is_event_buffer(lua_State* L, int idx) {
	int ret;
	lua_getmetatable(L, idx);
	luaL_getmetatable(L, EVENT_BUFFER_MT);
	ret = lua_rawequal(L, -2, -1);
	lua_pop(L, 2);
	return ret;
}

/* LUA: buffer:add(...)
	progressively adds items to the buffer
		if arg[*] is string, treat as a string:format call
		if arg[*] is a buffer, perform event_add_buffer
	returns number of bytes added
*/
int event_buffer_add(lua_State* L) {
	le_buffer* buf = event_buffer_get(L, 1);
	struct evbuffer* buffer = buf->buffer;
	int oldLength = EVBUFFER_LENGTH(buffer);
	int last = lua_top(L);
	int i;
	for(i = 2; i <= last; i++) {
		if(!lua_isstring(L, i) && !is_event_buffer(L, i))
			luaL_argerror(L, i, "Argument is not a string or buffer object");
/* Optionally perform checks and data loading separately to avoid overfilling the buffer */
#if BUFFER_ADD_CHECK_INPUT_FIRST
	}
	for(i = 2; i <= last; i++) {
#endif
		if(lua_isstring(L, i)) {
			size_t len;
			const char* data = lua_tolstring(L, i, &len);
			if(0 != evbuffer_add(buffer, data, len))
				luaL_error(L, "Failed to add data to the buffer");
		} else {
			le_buffer* buf2 = event_buffer_get(L, i);
			if(0 != evbuffer_add_buffer(buffer, buf2->buffer))
				luaL_error(L, "Failed to move buffer-data to the buffer");
		}
	}
	lua_pushinteger(L, EVBUFFER_LENGTH(buffer) - oldLength);
	return 1;
}

static luaL_Reg buffer_funcs[] = {
	{"add",event_buffer_add},
	{NULL, NULL}
};
static luaL_Ref funcs[] = {
	{NULL, NULL}
};
 
int event_buffer_register(lua_State* L) {
	luaL_newmetatable(L, EVENT_BUFFER_MT);
	lua_newtable(L);
	luaL_register(L, NULL, buffer_funcs);
	lua_setfield(L, -2, "__index");
	lua_pop(L, 1);
	
	luaL_register(L, "luaevent.core.buffer", funcs);
	return 0;
}
 
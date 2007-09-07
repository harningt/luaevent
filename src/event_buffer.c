/* LuaEvent - Copyright (C) 2007 Thomas Harning <harningt@gmail.com>
 * Licensed as LGPL - See doc/COPYING for details */

#include "event_buffer.h"
#include <lauxlib.h>

#define EVENT_BUFFER_MT "EVENT_BUFFER_MT"

#define BUFFER_ADD_CHECK_INPUT_FIRST 1

static le_buffer* event_buffer_get(lua_State* L, int idx) {
	return (le_buffer*)luaL_checkudata(L, idx, EVENT_BUFFER_MT);
}
static le_buffer* event_buffer_check(lua_State* L, int idx) {
	le_buffer* buf = (le_buffer*)luaL_checkudata(L, idx, EVENT_BUFFER_MT);
	if(!buf->buffer)
		luaL_argerror(L, idx, "Attempt to use closed event_buffer object");
	return buf;
}
static int is_event_buffer(lua_State* L, int idx) {
	int ret;
	lua_getmetatable(L, idx);
	luaL_getmetatable(L, EVENT_BUFFER_MT);
	ret = lua_rawequal(L, -2, -1);
	lua_pop(L, 2);
	return ret;
}

/* TODO: Use lightuserdata mapping to locate hanging object instances */
static int event_buffer_push(lua_State* L, struct evbuffer* buffer) {
	le_buffer *buf = (le_buffer*)lua_newuserdata(L, sizeof(le_buffer));
	buf->buffer = buffer;
	luaL_getmetatable(L, EVENT_BUFFER_MT);
	lua_setmetatable(L, -2);
	return 1;
}

static int event_buffer_push_new(lua_State* L) {
	return event_buffer_push(L, evbuffer_new());
}

static int event_buffer_gc(lua_State* L) {
	le_buffer* buf = event_buffer_get(L, 1);
	if(buf->buffer) {
		evbuffer_free(buf->buffer);
		buf->buffer = NULL;
	}
	return 0;
}

/* LUA: buffer:add(...)
	progressively adds items to the buffer
		if arg[*] is string, treat as a string:format call
		if arg[*] is a buffer, perform event_add_buffer
	returns number of bytes added
*/
static int event_buffer_add(lua_State* L) {
	le_buffer* buf = event_buffer_check(L, 1);
	struct evbuffer* buffer = buf->buffer;
	int oldLength = EVBUFFER_LENGTH(buffer);
	int last = lua_gettop(L);
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
			le_buffer* buf2 = event_buffer_check(L, i);
			if(0 != evbuffer_add_buffer(buffer, buf2->buffer))
				luaL_error(L, "Failed to move buffer-data to the buffer");
		}
	}
	lua_pushinteger(L, EVBUFFER_LENGTH(buffer) - oldLength);
	return 1;
}

static int event_buffer_get_length(lua_State* L) {
	le_buffer* buf = event_buffer_check(L, 1);
	lua_pushinteger(L, EVBUFFER_LENGTH(buf->buffer));
	return 1;
}

/* MAYBE: Could add caching */
static int event_buffer_get_data(lua_State* L) {
	le_buffer* buf = event_buffer_check(L, 1);
	lua_pushlstring(L, (const char*)EVBUFFER_DATA(buf->buffer), EVBUFFER_LENGTH(buf->buffer));
	return 1;
}

static int event_buffer_drain(lua_State* L) {
	le_buffer* buf = event_buffer_check(L, 1);
	size_t len = luaL_checkinteger(L, 2);
	evbuffer_drain(buf->buffer, len);
	return 0;
}

static luaL_Reg buffer_funcs[] = {
	{"add",event_buffer_add},
	{"length",event_buffer_get_length},
	{"get_data",event_buffer_get_data},
	{"drain",event_buffer_drain},
	{"close",event_buffer_gc},
	{NULL, NULL}
};
static luaL_Reg funcs[] = {
	{"new",event_buffer_push_new},
	{NULL, NULL}
};
 
int event_buffer_register(lua_State* L) {
	luaL_newmetatable(L, EVENT_BUFFER_MT);
	lua_pushcfunction(L, event_buffer_gc);
	lua_setfield(L, -2, "__gc");
	lua_newtable(L);
	luaL_register(L, NULL, buffer_funcs);
	lua_setfield(L, -2, "__index");
	lua_pop(L, 1);
	
	luaL_register(L, "luaevent.core.buffer", funcs);
	return 0;
}

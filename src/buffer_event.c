/* LuaEvent - Copyright (C) 2007 Thomas Harning <harningt@gmail.com>
 * Licensed as LGPL - See doc/COPYING for details */
#include "buffer_event.h"
#include "luaevent.h"
#include "utility.h"
#include <lauxlib.h>
#include <malloc.h>
#include "event_buffer.h"

#define BUFFER_EVENT_MT "BUFFER_EVENT_MT"

/* Obtains an le_bufferevent structure from a given index */
static le_bufferevent* buffer_event_get(lua_State* L, int idx) {
	return (le_bufferevent*)luaL_checkudata(L, idx, BUFFER_EVENT_MT);
}

/* Obtains an le_bufferevent structure from a given index
	AND checks that it hadn't been prematurely freed
*/
le_bufferevent* buffer_event_check(lua_State* L, int idx) {
	le_bufferevent* buf = (le_bufferevent*)luaL_checkudata(L, idx, BUFFER_EVENT_MT);
	if(!buf->ev)
		luaL_argerror(L, idx, "Attempt to use closed buffer_event object");
	return buf;
}

/* Checks if the given index contains an le_buffer object */
int is_buffer_event(lua_State* L, int idx) {
	int ret;
	lua_getmetatable(L, idx);
	luaL_getmetatable(L, BUFFER_EVENT_MT);
	ret = lua_rawequal(L, -2, -1);
	lua_pop(L, 2);
	return ret;
}

static void handle_callback(le_bufferevent* le_ev, short what, int callbackIndex) {
	lua_State* L = le_ev->base->loop_L;
	le_weak_get(L, le_ev);
	lua_getfenv(L, -1);
	lua_rawgeti(L, -1, callbackIndex);
	lua_remove(L, -2);
	lua_pushvalue(L, -2);
	lua_remove(L, -3);
	/* func, bufferevent */
	lua_pushinteger(L, what);
	/* What to do w/ errors...? */
	lua_pcall(L, 3, 0, 0);
}

static void buffer_event_readcb(struct bufferevent *ev, void *ptr) {
	handle_callback((le_bufferevent*)ptr, EVBUFFER_READ, 1);
}

static void buffer_event_writecb(struct bufferevent *ev, void *ptr) {
	handle_callback((le_bufferevent*)ptr, EVBUFFER_WRITE, 2);
}

static void buffer_event_errorcb(struct bufferevent *ev, short what, void *ptr) {
	handle_callback((le_bufferevent*)ptr, what, 3);
}

/* LUA: new(fd, read, write, error)
	Pushes a new bufferevent instance on the stack
	Accepts: base, fd, read, write, error cb
	Requires base, fd and error cb
*/
static int buffer_event_push(lua_State* L) {
	le_bufferevent *ev;
	le_base* base = event_base_get(L, 1);
	/* NOTE: Should probably reference the socket as well... */
	int fd = getSocketFd(L, 2);
	luaL_checktype(L, 5, LUA_TFUNCTION);
	if(!lua_isnil(L, 3)) luaL_checktype(L, 3, LUA_TFUNCTION);
	if(!lua_isnil(L, 4)) luaL_checktype(L, 4, LUA_TFUNCTION);
	ev= (le_bufferevent*)lua_newuserdata(L, sizeof(le_bufferevent));
	luaL_getmetatable(L, BUFFER_EVENT_MT);
	lua_setmetatable(L, -2);
	ev->ev = bufferevent_new(fd, buffer_event_readcb, buffer_event_writecb, buffer_event_errorcb, ev);
	lua_createtable(L, 5, 0);
	lua_pushvalue(L, 3);
	lua_rawseti(L, -2, 1); // Read
	lua_pushvalue(L, 4);
	lua_rawseti(L, -2, 2); // Write
	lua_pushvalue(L, 5);
	lua_rawseti(L, -2, 3); // Err

	event_buffer_push(L, ev->ev->input);
	lua_rawseti(L, -2, 4);
	event_buffer_push(L, ev->ev->output);
	lua_rawseti(L, -2, 5);
	lua_setfenv(L, -2);
	ev->base = base;
	return 1;
}

/* LUA: __gc and buffer:close()
	Releases the buffer resources
*/
static int buffer_event_gc(lua_State* L) {
	le_bufferevent* ev = buffer_event_get(L, 1);
	if(ev->ev) {
		le_buffer *read, *write;
		bufferevent_free(ev->ev);
		ev->ev = NULL;
		/* Also clear out the associated input/output event_buffers
		 * since they would have already been freed.. */
		lua_getfenv(L, 1);
		lua_rawgeti(L, -1, 4);
		lua_rawgeti(L, -2, 5);
		read = event_buffer_check(L, -2);
		write = event_buffer_check(L, -1);
		/* Erase their knowledge of the buffer so that the GC won't try to double-free */
		read->buffer = NULL;
		write->buffer = NULL;
	}
	return 0;
}

static luaL_Reg buffer_event_funcs[] = {
	{NULL, NULL}
};

static luaL_Reg funcs[] = {
	{"new", buffer_event_push},
	{NULL, NULL}
};

int buffer_event_register(lua_State* L) {
	luaL_newmetatable(L, BUFFER_EVENT_MT);
	lua_pushcfunction(L, buffer_event_gc);
	lua_setfield(L, -2, "__gc");
	lua_newtable(L);
	luaL_register(L, NULL, buffer_event_funcs);
	lua_setfield(L, -2, "__index");
	lua_pop(L, 1);

	luaL_register(L, "luaevent.core.bufferevent", funcs);
	return 1;
}

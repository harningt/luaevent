/* LuaEvent - Copyright (C) 2007 Thomas Harning <harningt@gmail.com>
 * Licensed as LGPL - See doc/COPYING for details */
#include "event_callback.h"
#include <assert.h>
#include <lauxlib.h>

#define EVENT_CALLBACK_ARG_MT "EVENT_CALLBACK_ARG_MT"

void freeCallbackArgs(le_callback* arg, lua_State* L) {
	if(arg->base) {
		arg->base = NULL;
		event_del(&arg->ev);
		luaL_unref(L, LUA_REGISTRYINDEX, arg->callbackRef);
	}
}
/* le_callback is allocated at the beginning of the coroutine in which it
is used, no need to manually de-allocate */

/* Index for coroutine is fd as integer for *nix, as lightuserdata for Win */
void luaevent_callback(int fd, short event, void* p) {
	le_callback* arg = p;
	lua_State* L;
	int ret;
	assert(arg && arg->base && arg->base->loop_L);
	L = arg->base->loop_L;
	lua_rawgeti(L, LUA_REGISTRYINDEX, arg->callbackRef);
	lua_pushinteger(L, event);
	lua_call(L, 1, 1);
	ret = lua_tointeger(L, -1);
	lua_pop(L, 1);
	if(ret == -1) {
		freeCallbackArgs(arg, L);
	} else {
		struct event *ev = &arg->ev;
		int newEvent = ret;
		if(newEvent != event) { // Need to hook up new event...
			event_del(ev);
			event_set(ev, fd, EV_PERSIST | newEvent, luaevent_callback, arg);
			event_add(ev, NULL);
		}
	}
}

static int luaevent_cb_gc(lua_State* L) {
	le_callback* arg = luaL_checkudata(L, 1, EVENT_CALLBACK_ARG_MT);
	freeCallbackArgs(arg, L);
	return 0;
}

le_callback* event_callback_push(lua_State* L, int baseIdx, int callbackIdx) {
	le_callback* cb;
	le_base *base = event_base_get(L, baseIdx);
	luaL_checktype(L, callbackIdx, LUA_TFUNCTION);
	cb = lua_newuserdata(L, sizeof(*cb));
	luaL_getmetatable(L, EVENT_CALLBACK_ARG_MT);
	lua_setmetatable(L, -2);

	lua_pushvalue(L, callbackIdx);
	cb->callbackRef = luaL_ref(L, LUA_REGISTRYINDEX);
	cb->base = base;
	return cb;
}

int event_callback_register(lua_State* L) {
	luaL_newmetatable(L, EVENT_CALLBACK_ARG_MT);
	lua_pushcfunction(L, luaevent_cb_gc);
	lua_setfield(L, -2, "__gc");
	lua_newtable(L);
	lua_pushcfunction(L, luaevent_cb_gc);
	lua_setfield(L, -2, "close");
	lua_setfield(L, -2, "__index");
	lua_pop(L, 1);
	return 0;
}

/* LuaEvent - Copyright (C) 2007 Thomas Harning <harningt@gmail.com>
 * Licensed as LGPL - See doc/COPYING for details */
 #include "luaevent.h"

#include <lua.h>
#include <lauxlib.h>
#include <assert.h>

#define EVENT_BASE_MT "EVENT_BASE_MT"
#define EVENT_CALLBACK_ARG_MT "EVENT_CALLBACK_ARG_MT"
#define MAIN_THREAD_LOCATION 1

void setMainThread(lua_State* L) {
	lua_pushthread(L);
	lua_rawseti(L, LUA_ENVIRONINDEX, MAIN_THREAD_LOCATION);
}
lua_State* getMainThread(lua_State* L) {
	lua_State* g_L;
	lua_rawgeti(L, LUA_ENVIRONINDEX, MAIN_THREAD_LOCATION);
	g_L = lua_tothread(L, -1);
	lua_pop(L, 1);
	return g_L;
}

int luaevent_newbase(lua_State* L) {
	le_base *base = (le_base*)lua_newuserdata(L, sizeof(le_base));
	base->loop_L = NULL; /* No running loop */
	base->base = event_init();
	luaL_getmetatable(L, EVENT_BASE_MT);
	lua_setmetatable(L, -2);
	return 1;
}

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
static void luaevent_callback(int fd, short event, void* p) {
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

static int luaevent_base_gc(lua_State* L) {
	le_base *base = luaL_checkudata(L, 1, EVENT_BASE_MT);
	if(base->base) {
		event_base_free(base->base);
		base->base = NULL;
	}
	return 0;
}

static int luaevent_cb_gc(lua_State* L) {
	le_callback* arg = luaL_checkudata(L, 1, EVENT_CALLBACK_ARG_MT);
	freeCallbackArgs(arg, L);
	return 0;
}

int getSocketFd(lua_State* L, int idx) {
	int fd;
	luaL_checktype(L, idx, LUA_TUSERDATA);
	lua_getfield(L, idx, "getfd");
	if(lua_isnil(L, -1))
		return luaL_error(L, "Socket type missing 'getfd' method");
	lua_pushvalue(L, idx);
	lua_call(L, 1, 1);
	fd = lua_tointeger(L, -1);
	lua_pop(L, 1);
	return fd;
}

/* sock, event, callback */
static int luaevent_addevent(lua_State* L) {
	int fd, event, callbackRef;
	le_callback* arg;
	le_base *base = luaL_checkudata(L, 1, EVENT_BASE_MT);
	fd = getSocketFd(L, 2);
	event = luaL_checkinteger(L, 3);
	luaL_checktype(L, 4, LUA_TFUNCTION);
	lua_pushvalue(L, 4);
	callbackRef = luaL_ref(L, LUA_REGISTRYINDEX);
	arg = lua_newuserdata(L, sizeof(*arg));
	luaL_getmetatable(L, EVENT_CALLBACK_ARG_MT);
	lua_setmetatable(L, -2);
	
	arg->base = base;
	arg->callbackRef = callbackRef;
	/* Setup event... */
	event_set(&arg->ev, fd, event | EV_PERSIST, luaevent_callback, arg);
	event_base_set(base->base, &arg->ev);
	event_add(&arg->ev, NULL);
	return 1;
}

static int luaevent_loop(lua_State* L) {
	le_base *base = luaL_checkudata(L, 1, EVENT_BASE_MT);
	base->loop_L = L;
	int ret = event_base_loop(base->base, 0);
	lua_pushinteger(L, ret);
	return 1;
}

static luaL_Reg base_funcs[] = {
	{ "addevent", luaevent_addevent },
	{ "loop", luaevent_loop },
	{ NULL, NULL }
};

static luaL_Reg funcs[] = {
	{ "new", luaevent_newbase },
	{ NULL, NULL }
};

typedef struct {
	const char* name;
	int value;
} namedInteger;

static namedInteger consts[] = {
	{"LEAVE", -1},
	{"EV_READ", EV_READ},
	{"EV_WRITE", EV_WRITE},
	{NULL, 0}
};

void setNamedIntegers(lua_State* L, namedInteger* p) {
	while(p->name) {
		lua_pushinteger(L, p->value);
		lua_setfield(L, -2, p->name);
		p++;
	}
}

/* Verified ok */
int luaopen_luaevent_core(lua_State* L) {
	/* Setup environ table */
	lua_createtable(L, 1, 0);
	lua_replace(L, LUA_ENVIRONINDEX);
	/* Setup metatable */
	luaL_newmetatable(L, EVENT_BASE_MT);
	lua_newtable(L);
	luaL_register(L, NULL, base_funcs);
	lua_setfield(L, -2, "__index");
	lua_pushcfunction(L, luaevent_base_gc);
	lua_setfield(L, -2, "__gc");
	lua_pop(L, 1);
	luaL_newmetatable(L, EVENT_CALLBACK_ARG_MT);
	lua_pushcfunction(L, luaevent_cb_gc);
	lua_setfield(L, -2, "__gc");
	lua_newtable(L);
	lua_pushcfunction(L, luaevent_cb_gc);
	lua_setfield(L, -2, "close");
	lua_setfield(L, -2, "__index");
	lua_pop(L, 1);

	luaL_register(L, "luaevent.core", funcs);
	setNamedIntegers(L, consts);
	return 1;
}

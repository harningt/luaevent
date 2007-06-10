#include "luaevent.h"

#include <lua.h>
#include <lauxlib.h>

#define EVENT_BASE_MT "EVENT_BASE_MT"
#define EVENT_CALLBACK_ARG_MT "EVENT_CALLBACK_ARG_MT"
#define EVENT_BASE_LOCATION 1

void setEventBase(lua_State* L, struct event_base* base) {
	struct event_base** pbase = lua_newuserdata(L, sizeof(base));
	*pbase = base;
	luaL_getmetatable(L, EVENT_BASE_MT);
	lua_setmetatable(L, -2);
	lua_rawseti(L, LUA_ENVIRONINDEX, EVENT_BASE_LOCATION);
}
struct event_base* getEventBase(lua_State* L) {
	struct event_base* base;
	lua_rawgeti(L, LUA_ENVIRONINDEX, EVENT_BASE_LOCATION);
	base = *(struct event_base**)lua_topointer(L, -1);
	lua_pop(L, 1);
	return base;
}

void freeCallbackArgs(le_callback* arg) {
	if(arg->L) {
		lua_State* L = arg->L;
		arg->L = NULL;
		event_del(&arg->ev);
		luaL_unref(L, LUA_REGISTRYINDEX, arg->callbackRef);
	}
}
/* le_callback is allocated at the beginning of the coroutine in which it
is used, no need to manually de-allocate */

/* Index for coroutine is fd as integer for *nix, as lightuserdata for Win */
static void luaevent_callback(int fd, short event, void* p) {
	le_callback* arg = p;
	lua_State* L = arg->L;
	int ret;
	lua_rawgeti(L, LUA_REGISTRYINDEX, arg->callbackRef);
	lua_pushinteger(L, event);
	lua_call(L, 1, 1);
	ret = lua_tointeger(L, -1);
	lua_pop(L, 1);
	if(ret == -1) {
		freeCallbackArgs(arg);
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
	struct event_base** pbase = luaL_checkudata(L, 1, EVENT_BASE_MT);
	if(*pbase) {
		event_base_free(*pbase);
		*pbase = NULL;
	}
	return 0;
}

static int luaevent_cb_gc(lua_State* L) {
	le_callback* arg = luaL_checkudata(L, 1, EVENT_CALLBACK_ARG_MT);
	freeCallbackArgs(arg);
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

/* Expected to be called at the beginning of the coro that uses it.. 
Value must be kept until coro is complete....
*/
/* sock, event, callback */
static int luaevent_addevent(lua_State* L) {
	int fd, event, callbackRef;
	le_callback* arg;
	fd = getSocketFd(L, 1);
	event = luaL_checkinteger(L, 2);
	luaL_checktype(L, 3, LUA_TFUNCTION);
	lua_pushvalue(L, 3);
	callbackRef = luaL_ref(L, LUA_REGISTRYINDEX);
	arg = lua_newuserdata(L, sizeof(*arg));
	luaL_getmetatable(L, EVENT_CALLBACK_ARG_MT);
	lua_setmetatable(L, -2);
	
	arg->L = L;
	arg->callbackRef = callbackRef;
	/* Setup event... */
	event_set(&arg->ev, fd, event | EV_PERSIST, luaevent_callback, arg);
	event_base_set(getEventBase(L), &arg->ev);
	event_add(&arg->ev, NULL);
	return 1;
}

static int luaevent_loop(lua_State* L) {
	int ret = event_base_loop(getEventBase(L), 0);
	lua_pushinteger(L, ret);
	return 1;
}

static luaL_Reg funcs[] = {
	{ "addevent", luaevent_addevent },
	{ "loop", luaevent_loop },
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
	lua_pushcfunction(L, luaevent_base_gc);
	lua_setfield(L, -2, "__gc");
	lua_pop(L, 1);
	luaL_newmetatable(L, EVENT_CALLBACK_ARG_MT);
	lua_pushcfunction(L, luaevent_cb_gc);
	lua_setfield(L, -2, "__gc");
	lua_pop(L, 1);

	setEventBase(L, event_init());
	
	luaL_register(L, "luaevent.core", funcs);
	setNamedIntegers(L, consts);
	return 1;
}

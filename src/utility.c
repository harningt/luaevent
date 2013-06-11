/* LuaEvent
   Copyright (C) 2007,2012,2013 Thomas Harning <harningt@gmail.com>

   Permission is hereby granted, free of charge, to any person obtaining a copy
   of this software and associated documentation files (the "Software"), to deal
   in the Software without restriction, including without limitation the rights
   to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
   copies of the Software, and to permit persons to whom the Software is
   furnished to do so, subject to the following conditions:

   The above copyright notice and this permission notice shall be included in
   all copies or substantial portions of the Software.

   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
   IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
   FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
   AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
   LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
   OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
   THE SOFTWARE.
   */

#include "utility.h"
#include <lauxlib.h>

#define WEAK_REF_LOCATION le_register_utility

static void get_weakref_table(lua_State* L) {
	lua_pushlightuserdata(L, WEAK_REF_LOCATION);
	lua_gettable(L, LUA_REGISTRYINDEX);
}

void le_weak_ref(lua_State* L, void* ptr, int idx) {
	get_weakref_table(L);
	lua_pushlightuserdata(L, ptr);
	if(idx < 0) idx-=2;
	lua_pushvalue(L, idx);
	lua_settable(L, -3);
}
void le_weak_unref(lua_State* L, void* ptr) {
	get_weakref_table(L);
	lua_pushlightuserdata(L, ptr);
	lua_pushnil(L);
	lua_settable(L, -3);
}

void le_weak_get(lua_State* L, void* ptr) {
	get_weakref_table(L);
	lua_pushlightuserdata(L, ptr);
	lua_gettable(L, -2);
}

static void push_weak_table(lua_State* L, const char* mode) {
	lua_newtable(L);
	lua_createtable(L,0,1);
	lua_pushstring(L,mode);
	lua_setfield(L,-2,"__mode");
	lua_setmetatable(L,-2);
}

void le_register_utility(lua_State* L) {
	lua_pushlightuserdata(L, WEAK_REF_LOCATION);
	push_weak_table(L, "v");
	lua_settable(L, LUA_REGISTRYINDEX);
}

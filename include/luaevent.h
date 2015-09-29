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

#ifndef LUAEVENT_H
#define LUAEVENT_H

#include <lua.h>

/* Workarounds for Lua 5.2 and Lua 5.3 */
#if (LUA_VERSION_NUM >= 502)

#undef lua_equal
#define lua_equal(L,idx1,idx2) lua_compare(L, (idx1), (idx2), LUA_OPEQ)

#undef lua_getfenv
#define lua_getfenv lua_getuservalue
#undef lua_setfenv
#define lua_setfenv lua_setuservalue

#undef lua_objlen
#define lua_objlen lua_rawlen

#undef luaL_register
#define luaL_register(L, n, f) \
	{ if ((n) == NULL) luaL_setfuncs(L, f, 0); else luaL_newlib(L, f); }

#endif

#include <sys/types.h>
#ifdef _WIN32
#include <winsock2.h>
#else
#include <sys/time.h>
#endif
#include <event.h>

typedef struct {
	struct event_base* base;
	lua_State* loop_L;
	int errorMessage;
} le_base;

le_base* event_base_get(lua_State* L, int idx);
void load_timeval(double time, struct timeval *tv);
int getSocketFd(lua_State* L, int idx);

int luaopen_luaevent_core(lua_State* L);

#endif

--[[
   LuaEvent
   Copyright (C) 2007,2012,2013,2016,2018,2019 Thomas Harning <harningt@gmail.com>

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
]]
local _M = {}
local core = require("luaevent.core")

_M.core = core
_M._NAME = "luaevent"
_M._VERSION = "0.4.6"

local EV_READ = core.EV_READ
local EV_WRITE = core.EV_WRITE
local base = core.new()

local function getWrapper()
	local running = coroutine.running()
	return function(...)
		return select(2, coroutine.resume(running, ...))
	end
end
-- Weak keys.. the keys are the client sockets
local clientTable = setmetatable({}, {'__mode', 'kv'})
local function socketWait(sock, event)
	if not clientTable[sock] then clientTable[sock] = _M.addevent(sock, event, getWrapper()) end
	coroutine.yield(event)
end

function _M.addevent(...)
	return base:addevent(...)
end


function _M.send(sock, data, start, stop)
	local s, err
	local from = start or 1
	local sent = 0
	repeat
		from = from + sent
		s, err, sent = sock:send(data, from, stop)
		if s or err ~= "timeout" then return s, err, sent end
		socketWait(sock, EV_WRITE)
	until false
end
function _M.receive(sock, pattern, part)
	local s, err
	pattern = pattern or '*l'
	repeat
		s, err, part = sock:receive(pattern, part)
		if s or err ~= "timeout" then return s, err, part end
		socketWait(sock, EV_READ)
	until false
end
-- same as above but with special treatment when reading chunks,
-- unblocks on any data received.
function _M.receivePartial(client, pattern)
	local s, err, part
	pattern = pattern or "*l"
	repeat
		s, err, part = client:receive(pattern)
		if s or ( (type(pattern)=="number") and part~="" and part ~=nil ) or err ~= "timeout" then
			return s, err, part
		end
		socketWait(client, EV_READ)
	until false
end
function _M.connect(sock, ...)
	sock:settimeout(0)
	local ret, err = sock:connect(...)
	if ret or err ~= "timeout" then return ret, err end
	socketWait(sock, EV_WRITE)
	ret, err = sock:connect(...)
	if err == "already connected" then
		return 1
	end
	return ret, err
end
-- Deprecated..
function _M.flush(sock)
end
local function clientCoroutine(sock, handler)
	-- Figure out what to do ......
	return handler(sock)
end
local function handleClient(co, client, handler)
	local ok, res, event = coroutine.resume(co, client, handler)
end
local function serverCoroutine(sock, callback)
	local listenItem = _M.addevent(sock, EV_READ, getWrapper())
	repeat
		local event = coroutine.yield(EV_READ)
		-- Get new socket
		local client = sock:accept()
		if client then
			client:settimeout(0)
			local co = coroutine.create(clientCoroutine)
			handleClient(co, client, callback)
		end
	until false
end
function _M.addserver(sock, callback)
	local coro = coroutine.create(serverCoroutine)
	assert(coroutine.resume(coro, sock, callback))
end
function _M.addthread(func, ...)
	return coroutine.resume(coroutine.create(func), ...)
end
local _skt_mt = {__index = {
	connect = function(self, ...)
		return _M.connect(self.socket, ...)
	end,
	send = function (self, data)
		return _M.send(self.socket, data)
	end,
	
	receive = function (self, pattern)
		if (self.timeout==0) then
			return _M.receivePartial(self.socket, pattern)
  		end
		return _M.receive(self.socket, pattern)
	end,
	
	flush = function (self)
		return _M.flush(self.socket)
	end,
	
	settimeout = function (self,time)
		self.timeout=time
		return
	end,
	
	close = function(self)
		clientTable[self.socket]:close()
		self.socket:close()
	end
}}
function _M.wrap(sock)
	return setmetatable({socket = sock}, _skt_mt)
end
_M.loop = function(...) base:loop(...) end

return _M

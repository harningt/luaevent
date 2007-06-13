--[[
	LuaEvent - Copyright (C) 2007 Thomas Harning <harningt@gmail.com>
	Licensed as LGPL - See doc/COPYING for details.
]]
module("luaevent", package.seeall)
require("luaevent.core")

local EV_READ = luaevent.core.EV_READ
local EV_WRITE = luaevent.core.EV_WRITE
local fair = false

local hookedObjectMt = false

function send(sock, data, start, stop)
	local s, err
	local from = start or 1
	local sent = 0
	repeat
		from = from + sent
		s, err, sent = sock:send(data, from, stop)
		-- Add extra coro swap for fairness
		-- CURRENTLY DISABLED FOR TESTING......
		if fair and math.random(100) > 90 then
			coroutine.yield(EV_WRITE)
		end
		if s or err ~= "timeout" then return s, err, sent end
		coroutine.yield(EV_WRITE)
	until false
end
function receive(sock, pattern, part)
	local s, err
	pattern = pattern or '*l'
	repeat
		s, err, part = sock:receive(pattern, part)
		if s or err ~= "timeout" then return s, err, part end
		coroutine.yield(EV_READ)
	until false
end
-- same as above but with special treatment when reading chunks,
-- unblocks on any data received.
function receivePartial(client, pattern)
	local s, err, part
	pattern = pattern or "*l"
	repeat
	s, err, part = client:receive(pattern)
	if s or ( (type(pattern)=="number") and part~="" and part ~=nil ) or 
		err ~= "timeout" then return s, err, part end
		coroutine.yield(EV_READ)
	until false
end
function connect(sock, ...)
	sock:settimeout(0)
	local ret, err = sock:connect(...)
	if ret or err ~= "timeout" then return ret, err end
	coroutine.yield(EV_WRITE)
	ret, err = sock:connect(...)
	if err == "already connected" then
		return 1
	end
	return ret, err
end
-- Deprecated..
function flush(sock)
end
local function clientCoroutine(sock, handler)
	-- Figure out what to do ......
	return handler(sock)
end

local function serverCoroutine(sock, callback)
	repeat
		local event = coroutine.yield(EV_READ)
		-- Get new socket
		local client = sock:accept()
		if client then
			--cl[#cl + 1] = client
			client:settimeout(0)
			local coFunc = coroutine.wrap(clientCoroutine)
			luaevent.core.addevent(client, coFunc, client, callback)
		end
	until false
end

local oldAddEvent = luaevent.core.addevent
luaevent.core.addevent = function(...)
	local item = oldAddEvent(...)
	if not item then print("FAILED TO SETUP ITEM") return item end
	print("SETUP ITEM FOR: ", debug.getmetatable(item).getfd(item))
	if not hookedObjectMt then
		hookedObjectMt = true
		local mt = debug.getmetatable(item)
		local oldGC = mt.__gc
		mt.__gc = function(...)
			print("RELEASING ITEM FOR: ", mt.getfd(...))
			return oldGC(...)
		end
	end
	return item
end

function addserver(sock, callback)
	local coFunc = coroutine.wrap(serverCoroutine)
	luaevent.core.addevent(sock, coFunc, sock, callback)
end
function addthread(sock, func, ...)
	local coFunc = coroutine.wrap(func)
	luaevent.core.addevent(sock, coFunc, ...)
end
local _skt_mt = {__index = {
	connect = function(self, ...)
		return connect(self.socket, ...)
	end,
	send = function (self, data)
		return send (self.socket, data)
	end,
	
	receive = function (self, pattern)
		if (self.timeout==0) then
  			return receivePartial(self.socket, pattern)
  		end
		return receive (self.socket, pattern)
	end,
	
	flush = function (self)
		return flush (self.socket)
	end,
	
	settimeout = function (self,time)
		self.timeout=time
		return
	end,
	close = function(self)
		self.socket:close()
	end
}}
function wrap(sock)
	return setmetatable({socket = sock}, _skt_mt)
end
loop = luaevent.core.loop
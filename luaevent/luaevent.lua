module("luaevent", package.seeall)
require("luaevent.core")

local EV_READ = luaevent.core.EV_READ
local EV_WRITE = luaevent.core.EV_WRITE
local fair = false

-- Weak keys.. the keys are the client sockets
local clientTable = {} or setmetatable({}, {'__mode', 'k'})

local function getWrapper()
	local running = coroutine.running()
	return function(...)
		print(coroutine.running(), running)
		print(debug.traceback())
		if coroutine.running() == running then return end
		return select(2, coroutine.resume(running, ...))
	end
end

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
		if not clientTable[sock] then clientTable[sock] = luaevent.core.addevent(sock, EV_WRITE, getWrapper()) end
		coroutine.yield(EV_WRITE)
	until false
end
function receive(sock, pattern, part)
	local s, err
	pattern = pattern or '*l'
	repeat
		s, err, part = sock:receive(pattern, part)
		if s or err ~= "timeout" then return s, err, part end
		if not clientTable[sock] then clientTable[sock] = luaevent.core.addevent(sock, EV_READ, getWrapper()) end
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
		if not clientTable[sock] then clientTable[sock] = luaevent.core.addevent(sock, EV_READ, getWrapper()) end
		coroutine.yield(EV_READ)
	until false
end
function connect(sock, ...)
	sock:settimeout(0)
	local ret, err = sock:connect(...)
	if ret or err ~= "timeout" then return ret, err end
	if not clientTable[sock] then clientTable[sock] = luaevent.core.addevent(sock, EV_WRITE, getWrapper()) end
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
local function handleClient(co, client, handler)
	local ok, res, event = coroutine.resume(co, client, handler)
end
local function serverCoroutine(sock, callback)
	local listenItem = luaevent.core.addevent(sock, EV_READ, getWrapper())
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
function addserver(sock, callback)
	local coro = coroutine.create(serverCoroutine)
	assert(coroutine.resume(coro, sock, callback))
end
function addthread(func, ...)
	return coroutine.resume(coroutine.create(func), ...)
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
}}
function wrap(sock)
	return setmetatable({socket = sock}, _skt_mt)
end
loop = luaevent.core.loop
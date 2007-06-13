require"luaevent"
require"socket"
local function setupHook(thread)
	if not thread then debug.sethook(function(event) print("TRACE >: ", debug.getinfo(2, 'n').name) end, 'c')
	else debug.sethook(thread, function(event) print("TRACE ", thread,">: ", debug.getinfo(2, 'n').name) end, 'c') end
end
local count = 100
local function func(sock)
	sock = luaevent.wrap(sock)
	assert(sock:connect("localhost", 20000))
	for i = 1, 2 do
		local maxZ = 10
		for z = 1, maxZ do
			assert(sock:send("Greet me  "))
		end
		assert(sock:receive(10 * maxZ))
	end
	if skt then skt:close() end
	count = count - 1
	if count > 0 then
		--local sock = assert(socket.tcp())
		--luaevent.addthread(sock, func, sock)
	end
end
for i = 1, 500 do
	local sock = assert(socket.tcp())
	luaevent.addthread(sock, func, sock)
end
luaevent.loop()

require"luaevent"
require"socket"
local function setupHook(thread)
	if not thread then debug.sethook(function(event) print("TRACE >: ", debug.getinfo(2, 'n').name) end, 'c')
	else debug.sethook(thread, function(event) print("TRACE ", thread,">: ", debug.getinfo(2, 'n').name) end, 'c') end
end

local function func(sock)
	sock = luaevent.wrap(sock)
	assert(sock:connect("localhost", 20000))
	for i = 1, 10 do
		for z = 1, 100 do
			assert(sock:send("Greet me  "))
		end
		assert(sock:receive(10 * 100))
	end
end
for i = 1, 1020 do
	local sock = assert(socket.tcp())
	luaevent.addthread(sock, func, sock)
end
luaevent.loop()

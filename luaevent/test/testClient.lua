require"luaevent"
require"socket"

local function func()
	print("ACTIVATED")
	local sock = socket.tcp()
	--sock:
	sock = luaevent.wrap(sock)
	print(assert(sock:connect("localhost", 20000)))
	for i = 1, 100000 do assert(sock:send("Greet me  ")) assert(sock:receive(10)) collectgarbage() end
end

luaevent.addthread(func)

luaevent.loop()
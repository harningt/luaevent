-- Tests Copas with a simple Echo server
--
-- Run the test file and the connect to the server by telnet on the used port
-- to stop the test just send the command "quit"

require"luaevent"
require"socket"
local function echoHandler(skt)
	while true do
		local data,ret = luaevent.receive(skt, 10)
		if data == "quit" or ret == 'closed' or not data then
			break
		end
		--collectgarbage()
		if not luaevent.send(skt, data) then return end
	end
	if skt then skt:close() end
end

local server = assert(socket.bind("localhost", 20000))
server:settimeout(0)

luaevent.addserver(server, echoHandler)
luaevent.loop()